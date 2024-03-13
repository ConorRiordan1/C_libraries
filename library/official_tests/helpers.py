import socket
import struct
import ssl
import random
import string
from constants import *
import dijkstra

class response():

    def __init__(self, ret_code):
        self.ret_code = ret_code
        self.session_id = 0
        self.data_len = 0
        self.data = ""
        self.total_dist = 0

uid_location_dict = {}

def generate_unique(generation_function, existing_names, size=10):
    """Generates a unique identifier using the specified generation function
    that does not exist in the provided set of existing keys."""
    possible = generation_function(size)
    while possible in existing_names:
        possible = generation_function(size)
    return possible


def generate_alphanumeric(length=10):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def generate_alphanumeric_punctuation(length=10):
    return ''.join(random.choices(string.ascii_letters + string.digits + string.punctuation, k=length))

def generate_alpha(length=10):
    return ''.join(random.choices(string.ascii_letters, k=length))


def admin_login(socket_conn) -> int:
    return send_recv(socket_conn, OP_CODES["login"], 0, 5, 8, "adminpassword", 0).session_id

def set_up():
    ret_socket = None
    new_socket = socket.socket(IP_DOMAIN, socket.SOCK_STREAM)
    if ENCRYPTED:
        context = ssl.create_default_context()
        context.check_hostname = False
        context.verify_mode = ssl.CERT_NONE
        ret_socket = context.wrap_socket(new_socket)
    else:
        ret_socket = new_socket

    # set a 3 second timeout so we don't hang here
    ret_socket.settimeout(TIMEOUT)
    try:
        ret_socket.connect((IP_ADDRESS, PORT))
    except socket.timeout:
        print("[!] Timeout on connect()")
    ret_socket.settimeout(None)
    return ret_socket

def tear_down(client_sock):
    socket.close(client_sock)

def send_recv(host_socket: socket.socket, op_code, session_id, field1, field2, data, weight) -> response:
    host_socket.send(serialize(op_code, session_id, field1, field2, data, weight))
    # timeout just in case a student submission hangs
    host_socket.settimeout(TIMEOUT)
    response_data = b''
    while True:
        try:
            chunk = host_socket.recv(BUF_SIZE)
            response_data += chunk
            if not chunk or len(chunk) < BUF_SIZE:
                # no more data
                break
        except socket.timeout:
            break
    host_socket.settimeout(None)
    
    response_packet = deserialize(response_data, op_code)
    return response_packet

def just_send(host_socket: socket.socket, op_code, session_id, field1, field2, data, weight) -> response:
    host_socket.send(serialize(op_code, session_id, field1, field2, data, weight))
    # timeout just in case a student submission hangs
    host_socket.settimeout(TIMEOUT)
    

def serialize(op_code: int=0, session_id: int=0, field1: int=0, field2: int=0, data: str="", weight: int=0) -> bytes:
    data = str.encode(data)
    buffer = struct.pack(
        f"!BxxxLHH{len(data)}s", op_code, session_id, field1, field2, data
    )
    
    if weight:
        buffer += struct.pack("!H", weight)

    return buffer

def deserialize(buffer: bytes, request_type: int) -> response:
    """
    buffer to deserialize from
    request_type of request that was sent, this is required because some packets
    have different packet types on success
    """
    if len(buffer) == 0:
        return None
    response_data = response(struct.unpack("!B", buffer[:1])[0])

    if response_data.ret_code == RETURN_CODES["success"]:
        if request_type == OP_CODES["login"]:
            response_data.session_id = struct.unpack_from("!L", buffer, 1)[0]
        elif request_type == OP_CODES["list"] or request_type == OP_CODES["search"]:
            response_data.data_len = struct.unpack_from("!H", buffer, 1)[0]
            try:
                response_data.data = struct.unpack_from(f"!{response_data.data_len}s", buffer, 3)[0]
            except struct.error as e:
                print(f"Couldn't deserialize response: {e}")
        elif request_type == OP_CODES["route"]:
            (response_data.data_len, response_data.total_dist) = struct.unpack_from("!HxH", buffer, 1)
            response_data.data = struct.unpack_from(f"!{response_data.data_len}s", buffer, 6)[0]

    return response_data

def build_graph(filename:str) -> dijkstra.graph:
    ret_graph = dijkstra.Graph()
    node_count = 0
    with open(filename, "rb") as map_file:
        map_data = map_file.read()
        # probably don't need this
        timestamp = struct.unpack("!Q", map_data[:8])[0]
        offset = 8
        map_size = len(map_data)
        while offset < map_size:
            (new_uid, new_name_len, new_link_count) = struct.unpack_from("!HHH", map_data, offset)
            offset += 6
            new_name = struct.unpack_from(f"!{new_name_len}s", map_data, offset)[0].decode()
            uid_location_dict[new_name] = new_uid
            offset += new_name_len
            node_count += 1
            for _ in range(new_link_count):
                (weight, dest_uid) = struct.unpack_from(f"!HH", map_data, offset)
                offset += 4
                ret_graph.add_edge(new_uid, dest_uid, weight)

    print(f"[+] Built map with {node_count} nodes")
    return ret_graph
