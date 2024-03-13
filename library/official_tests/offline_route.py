from constants import *
from helpers import *
import argparse
import dijkstra
import shlex


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=str, help='map file')
    args = parser.parse_args()

    STAR_MAP = build_graph(args.file)

    while True:
        params = shlex.split(input("Input start and destination: "))
        try:
            start_uid = uid_location_dict[params[0]]
            dest_uid = uid_location_dict[params[1]]
            spf_map = dijkstra.DijkstraSPF(STAR_MAP, start_uid)
            print(f"Distance: {spf_map.get_distance(dest_uid)}")
            print(f"Route: {spf_map.get_path(dest_uid)}")
        except KeyError as e:
            print(e)
