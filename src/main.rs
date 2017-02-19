extern crate iron;
extern crate winapi;
extern crate kernel32;
extern crate user32;
extern crate advapi32;

use iron::prelude::*;
use iron::status;

#[repr(C)]
struct TokenPrivileges {
    privilege_count: winapi::DWORD,
    privileges: [winapi::winnt::LUID_AND_ATTRIBUTES; 1]
}

fn request_privileges() {
    let process = unsafe { kernel32::GetCurrentProcess() };
    let mut token = std::ptr::null_mut();
    unsafe {
        if advapi32::OpenProcessToken(process, winapi::winnt::TOKEN_ADJUST_PRIVILEGES | winapi::winnt::TOKEN_QUERY, &mut token) == 0 {
            panic!("Failed to obtain process token");
        }

        let mut luid: winapi::winnt::LUID = std::mem::zeroed();
        if advapi32::LookupPrivilegeValueA(std::ptr::null_mut(), "SeShutdownPrivilege".as_ptr() as *mut _, &mut luid) == 0 {
            panic!("Failed to find privilege");
        }

        let mut privileges: TokenPrivileges = std::mem::zeroed();
        privileges.privilege_count = 1;
        privileges.privileges[0].Attributes = winapi::winnt::SE_PRIVILEGE_ENABLED;
        privileges.privileges[0].Luid = luid;
        if advapi32::AdjustTokenPrivileges(token, 0, std::mem::transmute(&privileges), 0, std::ptr::null_mut(), std::ptr::null_mut()) == 0 {
            panic!("Failed to adjust token privileges");
        }
    }
}

fn sleep_machine() {
    request_privileges();
    unsafe { kernel32::SetSystemPowerState(0, 1); };
}

fn shutdown_machine() {
    request_privileges();
    unsafe {
        user32::ExitWindowsEx(
            winapi::winuser::EWX_SHUTDOWN | winapi::winuser::EWX_FORCEIFHUNG,
            winapi::reason::SHTDN_REASON_MAJOR_OPERATINGSYSTEM | winapi::reason::SHTDN_REASON_MINOR_UPGRADE | winapi::reason::SHTDN_REASON_FLAG_PLANNED
        );
    }
}

fn hide_console() {
    let window = unsafe { kernel32::GetConsoleWindow() };
    unsafe { user32::ShowWindow(window, winapi::SW_HIDE); };
}

fn main() {
    fn process(req: &mut Request) -> IronResult<Response> {
        match req.url.path[0].as_ref() {
            "sleep" => sleep_machine(),
            "shutdown" => shutdown_machine(),
            "stop" => std::process::exit(0),
            _ => ()
        };
        Ok(Response::with((status::Ok, "")))
    }

    hide_console();
    Iron::new(process).http("0.0.0.0:13666").unwrap();
}
