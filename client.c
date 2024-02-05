
#include "client.h"
// Private Function Protoypes:
static void     send_transactions(int           sockfd,
                                  transaction * trans_array,
                                  uint32_t      num_arrays,
                                  int32_t       trans_size);
static uint32_t client_line_count(char * argv[]);
static int      client_connect(uint32_t linecount, char * argv[]);

int
main(int argc, char * argv[])
{
    if (2 != argc)
    {
        printf("Usage: %s <infile>\n", argv[0]);

        argv[1] = "test.dat";
    }

    uint32_t linecount = client_line_count(argv);

    if (0 > linecount)
    {
        fprintf(stderr, "Error getting line count\n");

        return -1;
    }

    int client_process = client_connect(linecount, argv);

    if (0 > client_process)
    {
        fprintf(stderr, "Error sending packets\n");

        return -1;
    }

    return 0;
}

static void
send_transactions(int           sockfd,
                  transaction * trans_array,
                  uint32_t      num_arrays,
                  int32_t       trans_size)
{
    int32_t num_to_send = htonl(trans_size);

    if (-1 == send(sockfd, &num_to_send, sizeof(int32_t), 0))
    {
        fprintf(stderr, "Unable to send transactions");
        exit(EXIT_FAILURE);
    }

    ssize_t sent
        = send(sockfd, trans_array, trans_size * sizeof(transaction), 0);
    sleep(.1);
}

static uint32_t
client_line_count(char * argv[])
{
    FILE * file = fopen(argv[1], "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error opening file");

        return -1;
    }

    uint32_t linecount    = 0;
    int      current_char = 0;

    while ((current_char = fgetc(file)) != EOF)
    {

        if (current_char == '\n')
        {
            linecount++;
        }
    }

    linecount++; // final line
    fclose(file);
    file = NULL;

    return linecount;
}

static int
client_connect(uint32_t linecount, char * argv[])
{
    struct sockaddr_un addr;
    int                sfd;
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (-1 == sfd)
    {

        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un))
        == -1)
    {

        return -1;
    }

    FILE * p_input_file = fopen(argv[1], "r");

    if (NULL == p_input_file)
    {
        fprintf(stderr, "File Error");

        return -1;
    }

    uint32_t total_lines = linecount;
    uint32_t trans_size  = min(linecount, MAX_PACKET_SIZE);

    if (trans_size == MAX_PACKET_SIZE)
    {
        total_lines = total_lines - MAX_PACKET_SIZE;
    }
    else
    {
        total_lines = 0;
    }

    int32_t       total_balance[5] = { 0 };
    transaction * mytrans          = calloc(trans_size, sizeof(transaction));
    int current_line  = 0;
    int sleep_counter = 0;

    while (fscanf(p_input_file,
                  "%d %d",
                  &mytrans[current_line].account,
                  &mytrans[current_line].amount)
           == 2)
    {
        fflush(stdout);
        total_balance[(mytrans[current_line].account) - 1]
            += mytrans[current_line].amount;
        // Process the data or store it as needed
        mytrans[current_line].account = htonl(mytrans[current_line].account);
        mytrans[current_line].amount  = htonl(mytrans[current_line].amount);
        current_line++;

        if ((trans_size - 1 == current_line - 1)) // if at max packet size(300)
        {
            linecount = linecount - MAX_PACKET_SIZE;
            send_transactions(sfd, mytrans, linecount, trans_size);

            fflush(stdout);
            sleep_counter++;

            if (1 == sleep_counter)
            {
                sleep(.1);
                sleep_counter = 0;
            }

            current_line = 0;
            trans_size   = min(total_lines, MAX_PACKET_SIZE);

            if (trans_size == MAX_PACKET_SIZE)
            {

                total_lines = total_lines - MAX_PACKET_SIZE;
            }
            free(mytrans);
            mytrans = NULL;
            mytrans = calloc(trans_size, sizeof(transaction));
        }
    }

    linecount = 0;

    printf("\n");

    for (int idx = 0; idx < 5; idx++)
    {
        printf("index, %d, Total balance %d\n", idx, total_balance[idx]);
    }

    free(mytrans);
    printf("\n scan data success\n");
    fclose(p_input_file);
    close(sfd);

    return 0;
}
