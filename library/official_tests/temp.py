from helpers import *
from math import inf

starmap = build_graph("student_files/maps/small_map")
print(starmap)
reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}

for src_loc, src_uid in uid_location_dict.items():
    print(starmap.get_nodes())
    if src_uid not in starmap.get_nodes():
        # edge case for nodes that don't exist in the map
        continue
    spf_map = dijkstra.DijkstraSPF(starmap, src_uid)
    print("here")
    for dst_uid in starmap.get_nodes():
        dst_loc = reverse_uid_dict[dst_uid]
        if src_loc == dst_loc:
            # route to itself will be handled in another test
            continue
        print(f"src: {src_uid}, dst: {dst_uid}")
        expected_dist = spf_map.get_distance(dst_uid)
        if expected_dist != inf:
            uid_path = spf_map.get_path(dst_uid)
            expected_path = []
            for uid in uid_path:
                expected_path.append(reverse_uid_dict[uid])
            print(f"path: {expected_path}")
        else:
            print(f"no route")
        print(f"dist: {expected_dist}")