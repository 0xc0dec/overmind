#include "mongoose.h"
#include <iostream>
#include <string>
#include <iomanip>

#define LOG(msg) std::cout << mgs << std::endl;

class App
{
public:
    App(int port)
    {
        mg_mgr_init(&mgr, nullptr);
        address = std::to_string(port);
    }

    ~App()
    {
        mg_mgr_free(&mgr);
    }

    void run()
    {
        auto connection = mg_bind(&mgr, address.c_str(), handler);
        mg_set_protocol_http_websocket(connection);
        while (true)
            mg_mgr_poll(&mgr, 1000);
    }

private:
    static void handler(struct mg_connection *connection, int event, void *p)
    {
        if (event != MG_EV_HTTP_REQUEST)
            return;

        auto uri = get_request_uri(p);
        log(uri);

        if (uri == "/sleep")
        {
            write_response(connection, "200 OK");
            sleep_machine();
        }
        else
            write_response(connection, "501 Not Implemented");
    }

    static std::string get_request_uri(void *p)
    {
        auto msg = static_cast<struct http_message *>(p);
        std::string uri;
        uri.assign(msg->uri.p, msg->uri.len);
        return uri;
    }

    static void write_response(mg_connection* connection, const char* code_with_status)
    {
        mg_printf(connection, "HTTP/1.1 %s \r\nContent-Length: 0\r\n\r\n", code_with_status);
    }

    static void log(const std::string& msg)
    {
        auto time = std::time(nullptr);
        auto local_time = std::localtime(&time);
        std::cout << std::put_time(local_time, "%c") << "  " << msg << std::endl;
    }

    static void sleep_machine()
    {
        system("rundll32.exe powrprof.dll,SetSuspendState 0,1,0");
    }

    std::string address;
    mg_mgr mgr;
};

int main()
{
    App app(13666);
    app.run();
    return 0;
}