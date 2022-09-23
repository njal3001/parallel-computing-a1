#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <queue>
#include <omp.h>

using adjacency_matrix = std::vector<std::vector<size_t>>;

struct Troon;

struct Station {
    size_t id;
    std::string name;
    size_t popularity;
    Station *forward_stations[3];
    Station *backward_stations[3];

    Station(size_t id, const std::string& name, size_t popularity);

    friend std::ostream& operator<<(std::ostream& os, const Station& station);
};


struct Troon {
    enum Line {
        green = 0,
        yellow,
        blue,
    };

    enum class State {
        waiting_platform,
        on_platform,
        waiting_transit,
        in_transit,
    };

    enum class Direction {
        forward,
        backward,
    };

    size_t id;
    Direction direction;
    State state;
    size_t state_timestamp;
    Line line;
    const Station* on_station;


    Troon(size_t id, Direction direction, Line line, const Station *spawn_station);

    friend std::ostream& operator<<(std::ostream& os, const Troon& troon);
    friend std::ostream& operator<<(std::ostream& os, const Troon::Line& line);
};

struct CompareTroon {
    bool operator()(const Troon* a, const Troon* b) {
        if (a->state_timestamp != b->state_timestamp) {
            return a->state_timestamp > b->state_timestamp;
        }
        return a->id > b->id;
    }
};

struct Link {
    Station *from;
    Station *to;
    size_t length;
    std::priority_queue<Troon*, std::vector<Troon*>, CompareTroon> waiting_platform;
    Troon* on_platform;
    Troon* in_transit;
    omp_lock_t lock;

    Link(Station *from, Station *to, size_t length);
    ~Link();

    friend std::ostream& operator<<(std::ostream& os, const Link& link);
};

class Network {
private:
    std::vector<Troon> troons;
    std::vector<Station> stations;

    Station* start_stations[3];
    Station* end_stations[3];

    size_t ticks;
    size_t num_trains_to_spawn[3];
    size_t num_stations;
    size_t num_lines;
    std::vector<Link> links;
    std::vector<std::vector<Link*>> link_matrix;

public:
    Network(size_t num_stations,
              const std::vector<std::string>& station_names,
              const std::vector<size_t>& popularities,
              const adjacency_matrix& mat,
              const std::vector<std::string>& green_station_names,
              const std::vector<std::string>& yellow_station_names,
              const std::vector<std::string>& blue_station_names, size_t ticks,
              size_t num_green_trains, size_t num_yellow_trains,
              size_t num_blue_trains, size_t num_lines);

    void simulate();

private:
    void connect_stations(Troon::Line line, const std::vector<std::string>& line_names);
    void spawn_troons(size_t tick, Troon::Line line);
};
