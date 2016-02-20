#include "mongoose.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>

class App
{
public:
    App(int port)
    {
        mg_mgr_init(&mgr, nullptr);
        mgr.user_data = this;
        address = std::to_string(port);
    }

    ~App()
    {
        mg_mgr_free(&mgr);
        remove_from_tray();
    }

    void run()
    {
        auto connection = mg_bind(&mgr, address.c_str(), handle);
        mg_set_protocol_http_websocket(connection);
        hide_to_tray();
        while (!stop)
            mg_mgr_poll(&mgr, 1000);
    }

private:
    static void handle(struct mg_connection *connection, int event, void *p)
    {
        if (event != MG_EV_HTTP_REQUEST)
            return;

        auto uri = get_request_uri(p);
        log(uri);

        auto wnd_handle = GetConsoleWindow();
        
        if (uri == "/sleep")
        {
            write_response(connection, "200 OK");
            sleep_machine();
        }
        else if (uri == "/stop")
        {
            write_response(connection, "200 OK");
            static_cast<App*>(connection->mgr->user_data)->stop = true;
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

    void hide_to_tray()
    {
        auto window = GetConsoleWindow();

        tray_icon.cbSize = sizeof(tray_icon);
        tray_icon.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
        tray_icon.hWnd = window;
        strcpy(tray_icon.szTip, "Sleep server");
        tray_icon.uCallbackMessage = WM_LBUTTONDOWN;
        tray_icon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        tray_icon.uID = 1;
        Shell_NotifyIconA(NIM_ADD, &tray_icon);

        ShowWindow(window, SW_HIDE);
    }

    void remove_from_tray()
    {
        Shell_NotifyIconA(NIM_DELETE, &tray_icon);
    }

    NOTIFYICONDATAA tray_icon;
    std::string address;
    mg_mgr mgr;
    bool stop = false;
};

int main()
{
    App app(13666);
    app.run();
    return 0;
}