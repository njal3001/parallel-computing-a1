#include "network.h"
#include <iostream>
#include <sstream>
#include <assert.h>
#include <algorithm>

template <typename T> std::string to_string(const T& value) {
    std::stringstream os;
    os << value;
    return os.str();
}

Station::Station(size_t id, const std::string& name, size_t popularity) {
    this->id = id;
    this->name = name;
    this->popularity = popularity;

    for (size_t i = 0; i < 3; i++) {
        this->forward_stations[i] = nullptr;
        this->backward_stations[i] = nullptr;
    }
}

std::ostream& operator<<(std::ostream& os, const Station& station) {
    os << "(name: " << station.name <<", popularity: " << station.popularity <<
        ", id: " << station.id;

    for (size_t i = 0; i < 3; i++) {
        Station* forward_station = station.forward_stations[i];
        Troon::Line line = static_cast<Troon::Line>(i);
        if (forward_station) {
            os << ", " << line << " forward: " << forward_station->name;
        }
        Station* backward_station = station.backward_stations[i];
        if (backward_station) {
            os << ", " << line << " backward: " << backward_station->name;
        }
    }

    os << ")";

    return os;
}

Troon::Troon(size_t id, Direction direction, Line line, const Station *spawn_station ) {
    this->id = id;
    this->direction = direction;
    this->line = line;
    this->on_station = spawn_station;
    this->state = Troon::State::waiting_platform;
    this->state_timestamp = 0;
}

Link::Link(Station *from, Station *to, size_t length) {
    this->from = from;
    this->to = to;
    this->length = length;

    this->on_platform = nullptr;
    this->in_transit = nullptr;

    omp_init_lock(&this->lock);
}

Link::~Link() {
    omp_destroy_lock(&this->lock);
}

std::ostream& operator<<(std::ostream& os, const Link& link) {
    os << "Link: " << link.from->name << "->" << link.to->name << '\n';

    os << "\tOn platform: ";
    if (link.on_platform) os << *link.on_platform << '\n';
    else os << "None\n";

    os << "\tIn transit: ";
    if (link.in_transit) os << *link.in_transit << '\n';
    else os << "None\n";

    // std::priority_queue<Troon*, std::vector<Troon*>, CompareTroon> pq{link.waiting_platform};
    //
    // if (!pq.empty()) {
    //     os << "\tWaiting:\n";
    //     while (!pq.empty()) {
    //         auto waiting = pq.top();
    //         pq.pop();
    //         os << '\t' << *waiting << '\n';
    //     }
    // } else {
    //     os << "\tNo troons waiting\n";
    // }


    return os;
}

std::ostream& operator<<(std::ostream& os, const Troon::Line& line) {
    switch (line) {
        case Troon::Line::green:
            os << "green";
            break;
        case Troon::Line::yellow:
            os << "yellow";
            break;
        case Troon::Line::blue:
            os << "blue";
            break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Troon& troon) {
    switch (troon.line) {
        case Troon::Line::green:
            os << "g";
            break;
        case Troon::Line::yellow:
            os << "y";
            break;
        case Troon::Line::blue:
            os << "b";
            break;
    }

    os << troon.id << "-" << troon.on_station->name;
    if (troon.state == Troon::State::in_transit) {
        Station *next_station;
        if (troon.direction == Troon::Direction::forward) {
            next_station = troon.on_station->forward_stations[troon.line];
        } else {
            next_station = troon.on_station->backward_stations[troon.line];
        }

        os << "->" << next_station->name;
    } else if (troon.state == Troon::State::on_platform || troon.state == Troon::State::waiting_transit) {
        os << "%";
    } else if (troon.state == Troon::State::waiting_platform) {
        os << "#";
    }


// #ifdef DEBUG
//     switch (troon.state) {
//         case Troon::State::waiting_platform:
//         os << "  (Waiting platform";
//         break;
//         case Troon::State::on_platform:
//             os << "  (On platform";
//             break;
//         case Troon::State::waiting_transit:
//             os << "  (Waiting transit";
//             break;
//         case Troon::State::in_transit:
//             os << "  (In transit";
//             break;
//     }
//
//     os << ", ts" << troon.state_timestamp << ", ";
//
//     if (troon.direction == Troon::Direction::forward) {
//         os << "forward)";
//     } else {
//         os << "backward)";
//     }
//
// #endif
//
    return os;
}

Network::Network(size_t num_stations,
              const std::vector<std::string>& station_names,
              const std::vector<size_t>& popularities,
              const adjacency_matrix& mat,
              const std::vector<std::string>& green_station_names,
              const std::vector<std::string>& yellow_station_names,
              const std::vector<std::string>& blue_station_names, size_t ticks,
              size_t num_green_trains, size_t num_yellow_trains,
              size_t num_blue_trains, size_t num_lines)
    : ticks(ticks), num_stations(num_stations), num_lines(num_lines),
    link_matrix(num_stations, std::vector<Link*>(num_stations))
{
    this->num_trains_to_spawn[Troon::Line::green] = num_green_trains;
    this->num_trains_to_spawn[Troon::Line::yellow] = num_yellow_trains;
    this->num_trains_to_spawn[Troon::Line::blue] = num_blue_trains;

    // Reserve space for troons
    this->troons.reserve(num_green_trains + num_yellow_trains + num_blue_trains);

    // Create stations
    for (size_t i = 0; i < num_stations; i++) {
        Station station = Station(i, station_names[i], popularities[i]);
        this->stations.push_back(station);
    }

    // Create links
    for (size_t i = 0; i < num_stations; i++) {
        for (size_t j = 0; j < num_stations; j++) {
            size_t length = mat[i][j];
            if (length) {
                Station *from = &this->stations[i];
                Station *to = &this->stations[j];

                this->links.push_back(Link(from, to, length));
            }
        }
    }



    // Fill link matrix
    // NOTE: This can't be done when creating the links
    // because the links vector might be relocated
    for (auto &link : this->links) {
        this->link_matrix[link.from->id][link.to->id] = &link;
    }

    this->connect_stations(Troon::Line::green, green_station_names);
    this->connect_stations(Troon::Line::yellow, yellow_station_names);
    this->connect_stations(Troon::Line::blue, blue_station_names);

#ifdef DEBUG
    std::cout << "Stations:\n";
    for (const auto& s : this->stations) {
        std::cout << s << '\n';
    }

    std::cout << "Links:\n";
    for (const auto& l : this->links) {
        std::cout << l << '\n';
    }
#endif
}

void Network::connect_stations(Troon::Line line, const std::vector<std::string>& line_names)
{
    size_t num_line_stations = line_names.size();

    Station *head = nullptr;
    for (size_t i = 0; i < num_line_stations; i++) {
        for (size_t j = 0; j < this->num_stations; j++) {
            Station *station = &this->stations[j];
            if (line_names[i] == station->name) {
                if (head) {
                    head->forward_stations[line] = station;
                } else {
                    this->start_stations[line] = station;
                }

                station->backward_stations[line] = head;
                head = station;
            }
        }
    }

    this->end_stations[line] = head;
}

void Network::spawn_troons(size_t tick, Troon::Line line) {
    size_t left_to_spawn = this->num_trains_to_spawn[line];

    if (left_to_spawn > 0) {
        Station *start_station = this->start_stations[line];
        Station *start_next_station = start_station->forward_stations[line];

        this->troons.push_back(Troon(troons.size(), Troon::Direction::forward,
                    line, start_station));


        Link *front_link = this->link_matrix[start_station->id][start_next_station->id];

        this->troons.back().state_timestamp = tick;
        front_link->waiting_platform.push(&this->troons.back());

        left_to_spawn--;
        if (left_to_spawn > 0) {
            Station *end_station = this->end_stations[line];
            Station *end_next_station = end_station->backward_stations[line];

            this->troons.push_back(Troon(troons.size(), Troon::Direction::backward,
                        line, end_station));

            Link *end_link = this->link_matrix[end_station->id][end_next_station->id];

            this->troons.back().state_timestamp = tick;
            end_link->waiting_platform.push(&this->troons.back());

            left_to_spawn--;
        }

        this->num_trains_to_spawn[line] = left_to_spawn;
    }
}

void Network::simulate() {
    for (size_t tick = 0; tick < this->ticks; tick++) {

        // Spawn troons
        spawn_troons(tick, Troon::Line::green);
        spawn_troons(tick, Troon::Line::yellow);
        spawn_troons(tick, Troon::Line::blue);

        #pragma omp parallel for
        for (size_t i = 0; i < this->links.size(); i++) {
            Link& link = this->links[i];

            Troon *troon = link.in_transit;
            if (troon && tick - troon->state_timestamp >= link.length) {
                // Switch direction if terminal station has been reached
                if (troon->direction == Troon::Direction::forward
                        && !link.to->forward_stations[troon->line]) {
                    troon->direction = Troon::Direction::backward;
                }
                else if (troon->direction == Troon::Direction::backward
                        && !link.to->backward_stations[troon->line]) {
                    troon->direction = Troon::Direction::forward;
                }
                link.in_transit = nullptr;
                troon->state = Troon::State::waiting_platform;
                troon->state_timestamp = tick;
                troon->on_station = link.to;
                // Add to waiting area of next link
                Station *new_from = link.to;
                Station *new_to;
                if (troon->direction == Troon::Direction::forward) {
                    new_to = link.to->forward_stations[troon->line];
                } else {
                    new_to = link.to->backward_stations[troon->line];
                }
                Link *new_link = link_matrix[new_from->id][new_to->id];
                omp_set_lock(&new_link->lock);
                new_link->waiting_platform.push(troon);
                omp_unset_lock(&new_link->lock);
            }
        }
        #pragma omp parallel for
        for (size_t i = 0; i < this->links.size(); i++) {
            Link& link = this->links[i];
            // Move from platform to link
            if (link.on_platform) {
                if (link.on_platform->state == Troon::State::waiting_transit) {
                    link.on_platform->state = Troon::State::in_transit;
                    link.on_platform->state_timestamp = tick;
                    link.in_transit = link.on_platform;
                    link.on_platform = nullptr;
                }
                else {
                    if (!link.in_transit) {
                        // Check if troon is finished with opening and closing doors and
                        // letting passengers on
                        size_t wait_time = link.from->popularity + 2;

                        if (tick - link.on_platform->state_timestamp + 1 >= wait_time) {
                            link.on_platform->state = Troon::State::waiting_transit;
                            link.on_platform->state_timestamp = tick;
                        }
                    }
                }
            }
        }
        #pragma omp parallel for
        for (size_t i = 0; i < this->links.size(); i++) {
            Link& link = this->links[i];
            // Move from waiting area to platform
            // TODO: Wait for troons to arrive if needed
            if (!link.on_platform && !link.waiting_platform.empty()) {
                omp_set_lock(&link.lock);
                Troon* first_troon = link.waiting_platform.top();
                link.waiting_platform.pop();
                omp_unset_lock(&link.lock);

                link.on_platform = first_troon;
                first_troon->state_timestamp = tick;
                first_troon->state = Troon::State::on_platform;
            }
        }


#ifdef DEBUG
        std::cout << "\nState at tick " << tick << "\n\n";
        std::cout << "Troons:\n";
        for (const auto &troon : this->troons) {
            std::cout << troon << '\n';
        }

        std::cout << "Links:\n";
        for (const auto &link : this->links) {
            std::cout << link << '\n';
        }
#else
        if (this->ticks - tick <= this->num_lines) {
            // NOTE: Not sure if it matters,
            // but we could optimize this by
            // just keeping the troon container sorted.
            // Could use a priority queue for example.
            std::vector<std::string> v{};
            for (auto& troon: troons) {
                auto s = to_string(troon);
                v.emplace_back(s);
            }
            sort(v.begin(), v.end());
            std::cout << tick << ": ";
            for (const auto& element: v) {
                std::cout << element << " ";
            }

            std::cout << '\n';
        }
#endif
    }
}
