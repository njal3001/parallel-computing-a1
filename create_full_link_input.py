import random
import sys

MIN_LINK_LENGTH = 1
MAX_LINK_LENGTH = 10
MIN_POPULARITY = 1
MAX_POPULARITY = 5

GREEN_LINE = 0
YELLOW_LINE = 1
BLUE_LINE = 2
NUM_LINES = 3

class Station:
    def __init__(self, name, id, popularity):
        self.on_line = [False for i in range(NUM_LINES)]
        self.name = name
        self.id = id
        self.popularity = popularity

    def line_count(self):
        count = 0
        for l in self.on_line:
            if l:
                count += 1

        return count

# Command line arguments
assert(len(sys.argv) == 7)

NUM_STATIONS = int(sys.argv[1])
NUM_TICKS = int(sys.argv[2])
NUM_GREEN_TROONS = int(sys.argv[3])
NUM_YELLOW_TROONS = int(sys.argv[4])
NUM_BLUE_TROONS = int(sys.argv[5])
NUM_LINE_OUTPUT = int(sys.argv[6])

def number_to_ascii_id(num):
    num_str = str(num)
    id = ""
    for c in num_str:
        id += chr(ord(c) + 17)

    return id

stations = [Station(number_to_ascii_id(i), i,
    random.randrange(MIN_POPULARITY, MAX_POPULARITY + 1)) for i in range(NUM_STATIONS)]

adj_matrix = [[0 for j in range(NUM_STATIONS)] for i in range(NUM_STATIONS)]

line_stations = [[] for i in range(NUM_LINES)]

for line in range(NUM_LINES):
    for station in stations:
        station.on_line[line] = True
        if len(line_stations[line]) > 0:
            prev_station = line_stations[line][-1]

            link_length = adj_matrix[prev_station.id][station.id]
            if link_length == 0:
                link_length = random.randrange(MIN_LINK_LENGTH, MAX_LINK_LENGTH + 1)

            adj_matrix[prev_station.id][station.id] = link_length
            adj_matrix[station.id][prev_station.id] = link_length

        line_stations[line].append(station)

# Output
print(NUM_STATIONS)
for station in stations[:-1]:
    print(station.name, end=" ")
print(stations[-1].name)

for station in stations[:-1]:
    print(station.popularity, end=" ")
print(stations[-1].popularity)

for i in range(NUM_STATIONS):
    for j in range(NUM_STATIONS - 1):
        print(adj_matrix[i][j], end=" ")

    print(adj_matrix[i][NUM_STATIONS - 1])

for i in range(NUM_LINES):
    for station in line_stations[i][:-1]:
        print(station.name, end=" ")

    print(line_stations[i][-1].name)

print(NUM_TICKS)
print(f'{NUM_GREEN_TROONS} {NUM_YELLOW_TROONS} {NUM_BLUE_TROONS}')
print(NUM_LINE_OUTPUT)
