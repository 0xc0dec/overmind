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
        removeFromTray();
    }

    void run()
    {
        auto connection = mg_bind(&mgr, address.c_str(), handle);
        mg_set_protocol_http_websocket(connection);
        hideToTray();
        while (!stop)
            mg_mgr_poll(&mgr, 1000);
    }

private:
    static void handle(struct mg_connection *connection, int event, void *p)
    {
        if (event != MG_EV_HTTP_REQUEST)
            return;

        auto uri = getRequestUri(p);
        log(uri);

        if (uri == "/sleep")
        {
            writeResponse(connection, "200 OK");
            sleepMachine();
        }
        else if (uri == "/stop")
        {
            writeResponse(connection, "200 OK");
            static_cast<App*>(connection->mgr->user_data)->stop = true;
        }
        else
            writeResponse(connection, "501 Not Implemented");
    }

    static std::string getRequestUri(void *p)
    {
        auto msg = static_cast<struct http_message *>(p);
        std::string uri;
        uri.assign(msg->uri.p, msg->uri.len);
        return uri;
    }

    static void writeResponse(mg_connection* connection, const char* codeWithStatus)
    {
        mg_printf(connection, "HTTP/1.1 %s \r\nContent-Length: 0\r\n\r\n", codeWithStatus);
    }

    static void log(const std::string& msg)
    {
        auto time = std::time(nullptr);
        auto localTime = std::localtime(&time);
        std::cout << std::put_time(localTime, "%c") << "  " << msg << std::endl;
    }

    static void sleepMachine()
    {
        HANDLE token;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        {
            log("Failed to open process token");
            return;
        }

        TOKEN_PRIVILEGES priv;
        LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &priv.Privileges[0].Luid);

        priv.PrivilegeCount = 1;
        priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(token, false, &priv, 0, nullptr, nullptr))
        {
            log("Failed to adjust token privileges");
            return;
        }

        if (!SetSystemPowerState(false, true))
            log("Failed to put PC to sleep");
    }

    void hideToTray()
    {
        auto window = GetConsoleWindow();

        trayIcon.cbSize = sizeof(trayIcon);
        trayIcon.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
        trayIcon.hWnd = window;
        strcpy(trayIcon.szTip, "Sleep server");
        trayIcon.uCallbackMessage = WM_LBUTTONDOWN;
        trayIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        trayIcon.uID = 1;
        Shell_NotifyIconA(NIM_ADD, &trayIcon);

        ShowWindow(window, SW_HIDE);
    }

    void removeFromTray()
    {
        Shell_NotifyIconA(NIM_DELETE, &trayIcon);
    }

    NOTIFYICONDATAA trayIcon;
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