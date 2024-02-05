import socket
import ssl

hostname = 'localhost'
context = ssl.create_default_context(cafile="cert.pem")
with socket.create_connection((hostname, 4433)) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as ssock:
        print(ssock.version())

        # note, this will break if a non-utf8 byte is sent
        new_data = ssock.recv(1024).decode('utf-8')

        while True:
            print(f"Recv: {new_data}")
            send_data = ""
            while len(send_data) == 0:
                send_data = input("Send: ")
            my_data = send_data.encode('utf-8')
            ssock.send(my_data)
            new_data = ssock.recv(1024).decode('utf-8')
