#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <mysql.h>
#include <vector>
using namespace std;

#define BUFFER_LEN 512

typedef struct tagClient
{
    pthread_t t_handle;
    int socket_fd;
} CLIENT;

typedef struct tagDeviceID
{
    char string[5];
} DEVICEID;

vector<CLIENT>      g_vClient;
pthread_mutex_t     g_mClientManage;
MYSQL*              g_pMysqlConn;
pthread_mutex_t     g_mDatabase;

void error_msg(const char* message)
{
    printf("[Error] %s\n", message);
    exit(1);
}

void database_connect(void)
{
    g_pMysqlConn = mysql_init(NULL);
    if (!mysql_real_connect(g_pMysqlConn, "127.0.0.1", "root", "1234qwer", NULL, 0, NULL, 0))
    {
        error_msg("mysql connect error!");
    }
    else
    {
        if (mysql_select_db(g_pMysqlConn, "iot_project"))
        {
            error_msg("mysql select db error!");
        }
    }
}

void write_sensordata(DEVICEID* pId, float* fTemp, float* fPres, float* fHumi, unsigned long* nGasr, unsigned short* nAirs)
{
    int i;
    MYSQL_RES* result;
    MYSQL_ROW row;
    char strDate[32] = { 0, };
    char strQuery[256] = { 0, };

    // get date
    memset(&strQuery[0], 0, sizeof(strQuery));
    sprintf(strQuery, "select now() as DATE");
    pthread_mutex_lock(&g_mDatabase);
    if (mysql_query(g_pMysqlConn, strQuery) == 0)
    {
	    result = mysql_store_result(g_pMysqlConn);
	    while ((row = mysql_fetch_row(result)) != NULL)
        {
            strcpy(strDate, row[0]);
        }
	    mysql_free_result(result);
    }
    else
    {
	    pthread_mutex_unlock(&g_mDatabase);
        error_msg("[1] query failed!");
    }
    pthread_mutex_unlock(&g_mDatabase);

    for (i=0; i<2; i++)
    {
	    memset(&strQuery[0], 0, sizeof(strQuery));
	    sprintf(strQuery, "insert into sensor_table (temp, pres, hum, gas, score, id, regtime) value (%.2f, %.2f, %.2f, %d, %d, \"%s\", \"%s\");", fTemp[i], fPres[i], fHumi[i], nGasr[i], nAirs[i], pId[i].string, strDate);

	    pthread_mutex_lock(&g_mDatabase);
	    if (mysql_query(g_pMysqlConn, strQuery))
	    {
		    pthread_mutex_unlock(&g_mDatabase);
		    error_msg("[2] query failed!");
	    }
	    pthread_mutex_unlock(&g_mDatabase);
    }

    printf("write sensor data\n");
}

void* ClientThread(void* pClient)
{
    CLIENT client = *(CLIENT *)pClient;
    int ret;
    unsigned char buffer[BUFFER_LEN];
    vector<CLIENT>::iterator iter;

    while (1)
    {
        if ((ret = read(client.socket_fd, &buffer[0], BUFFER_LEN)) < 0)
        {
            printf("client [%d]: %d", client.socket_fd, ret);
            error_msg("read error");
        }
        else
        {
            if (ret == 0)
            {
                // disconnect
                printf("client [%d] disconnect\n", client.socket_fd);

                pthread_mutex_lock(&g_mClientManage);
                for (iter=g_vClient.begin(); iter!=g_vClient.end(); iter++)
                {
                    if (iter->socket_fd == client.socket_fd)
                        break;
                }
                g_vClient.erase(iter);
                pthread_mutex_unlock(&g_mClientManage);

                pthread_exit((void *)0);
            }
            else
            {
                // data processing
                printf("client [%d] data processing\n", client.socket_fd);

                if (buffer[0] == 'S' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'S')
                {
                    printf("got sensor data\n");

		            int i;
                    DEVICEID deviceId[2];
                    float fTemp[2];
                    float fPres[2];
                    float fHumi[2];
                    unsigned long nGasr[2];
                    unsigned short nAirs[2];
                    unsigned short u16Buf;
                    unsigned long u32Buf;

#if 0 // for debug
                    int i;
                    for (i=0; i<24; i++)
                    {
                    buffer[i] = buffer[i] & 0xFF;
                    printf("%02x ", buffer[i]);
                            }
                    printf("\n");
#endif

                    for (i=0; i<2/*buffer[4]*/; i++)
                    {
                        deviceId[i].string[0] = buffer[5 + (20 * i)];
                        deviceId[i].string[1] = buffer[6 + (20 * i)];
                        deviceId[i].string[2] = buffer[7 + (20 * i)];
                        deviceId[i].string[3] = buffer[8 + (20 * i)];
                        deviceId[i].string[4] = 0;

                        u16Buf = (unsigned short)((buffer[9 + (20 * i)] << 8) | (buffer[10 + (20 * i)]));
                        fTemp[i] = (float)u16Buf / 100.0f;

                        u32Buf = (unsigned long)((buffer[11 + (20 * i)] << 24) | (buffer[12 + (20 * i)] << 16) | (buffer[13 + (20 * i)] << 8) | (buffer[14 + (20 * i)]));
                        fPres[i] = (float)u32Buf / 100.0f;

                        u32Buf = (unsigned long)((buffer[15 + (20 * i)] << 24) | (buffer[16 + (20 * i)] << 16) | (buffer[17 + (20 * i)] << 8) | (buffer[18 + (20 * i)]));
                        fHumi[i] = (float)u32Buf / 1000.0f;

                        u32Buf = (unsigned long)((buffer[19 + (20 * i)] << 24) | (buffer[20 + (20 * i)] << 16) | (buffer[21 + (20 * i)] << 8) | (buffer[22 + (20 * i)]));
                        nGasr[i] = u32Buf;

                        u16Buf = (unsigned short)((buffer[23 + (20 * i)] << 8) | (buffer[24 + (20 * i)]));
                        nAirs[i] = u16Buf;
                    }

		            write_sensordata(&deviceId[0], &fTemp[0], &fPres[0], &fHumi[0], &nGasr[0], &nAirs[0]);
                }
            }
        }
    }
}

int main(void)
{
    int socket_fd, client_fd;
    sockaddr_in server_addr, client_addr;
    int addr_len;

    printf("==========================================\n");
    printf(" IoT TCP/IP Server\n");
    printf("==========================================\n");

    pthread_mutex_init(&g_mClientManage, NULL);
    pthread_mutex_init(&g_mDatabase, NULL);

    database_connect();

    // create server socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error_msg("can not create server socket");
        return -1;
    }
 
    // setting server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.68.20.20");
    server_addr.sin_port = htons(3360);
 
    // bind
    if (bind(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error_msg("bind error");
        return -1;
    }

    // listen
    if (listen(socket_fd, 10) < 0)
    {
        error_msg("listen error");
        return -1;
    }
    
    while(1)
    {
        CLIENT client;

        // accept
        addr_len = sizeof(client_addr);
        client_fd = accept(socket_fd, (sockaddr *)&client_addr, (socklen_t *)&addr_len);
        if (client_fd < 0)
        {
            error_msg("client socket is wrong");
            return -1;
        }

        client.socket_fd = client_fd;
        pthread_create(&client.t_handle, NULL, ClientThread, (void *)&client);

        pthread_mutex_lock(&g_mClientManage);
        g_vClient.push_back(client);
        pthread_mutex_unlock(&g_mClientManage);

        printf("New client! socket=%d thread=%d\n", client.socket_fd, client.t_handle);
    }

    mysql_close(g_pMysqlConn);
 
    return 0;
}
