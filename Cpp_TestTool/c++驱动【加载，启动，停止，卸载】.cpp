#if 0
#include <iostream>
#include <Windows.h>
using namespace std;
// 安装驱动
BOOL installDvr(CONST WCHAR drvPath[50], CONST WCHAR serviceName[20]) {

    // 打开服务控制管理器数据库
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                   // 目标计算机的名称,NULL：连接本地计算机上的服务控制管理器
        NULL,                   // 服务控制管理器数据库的名称，NULL：打开 SERVICES_ACTIVE_DATABASE 数据库
        SC_MANAGER_ALL_ACCESS   // 所有权限
    );
    if (schSCManager == NULL) {
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // 创建服务对象，添加至服务控制管理器数据库
    SC_HANDLE schService = CreateService(
        schSCManager,               // 服务控件管理器数据库的句柄
        serviceName,                // 要安装的服务的名称
        serviceName,                // 用户界面程序用来标识服务的显示名称
        SERVICE_ALL_ACCESS,         // 对服务的访问权限：所有全权限
        SERVICE_KERNEL_DRIVER,      // 服务类型：驱动服务
        SERVICE_DEMAND_START,       // 服务启动选项：进程调用 StartService 时启动
        SERVICE_ERROR_IGNORE,       // 如果无法启动：忽略错误继续运行
        drvPath,                    // 驱动文件绝对路径，如果包含空格需要多加双引号
        NULL,                       // 服务所属的负载订购组：服务不属于某个组
        NULL,                       // 接收订购组唯一标记值：不接收
        NULL,                       // 服务加载顺序数组：服务没有依赖项
        NULL,                       // 运行服务的账户名：使用 LocalSystem 账户
        NULL                        // LocalSystem 账户密码
    );
    if (schService == NULL) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// 启动服务
BOOL startDvr(CONST WCHAR serviceName[20]) {

    // 打开服务控制管理器数据库
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                   // 目标计算机的名称,NULL：连接本地计算机上的服务控制管理器
        NULL,                   // 服务控制管理器数据库的名称，NULL：打开 SERVICES_ACTIVE_DATABASE 数据库
        SC_MANAGER_ALL_ACCESS   // 所有权限
    );
    if (schSCManager == NULL) {
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // 打开服务
    SC_HANDLE hs = OpenService(
        schSCManager,           // 服务控件管理器数据库的句柄
        serviceName,            // 要打开的服务名
        SERVICE_ALL_ACCESS      // 服务访问权限：所有权限
    );
    if (hs == NULL) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    if (StartService(hs, 0, 0) == 0) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }


    CloseServiceHandle(hs);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// 停止服务
BOOL stopDvr(CONST WCHAR serviceName[20]) {

    // 打开服务控制管理器数据库
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                   // 目标计算机的名称,NULL：连接本地计算机上的服务控制管理器
        NULL,                   // 服务控制管理器数据库的名称，NULL：打开 SERVICES_ACTIVE_DATABASE 数据库
        SC_MANAGER_ALL_ACCESS   // 所有权限
    );
    if (schSCManager == NULL) {
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // 打开服务
    SC_HANDLE hs = OpenService(
        schSCManager,           // 服务控件管理器数据库的句柄
        serviceName,            // 要打开的服务名
        SERVICE_ALL_ACCESS      // 服务访问权限：所有权限
    );
    if (hs == NULL) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // 如果服务正在运行
    SERVICE_STATUS status;
    if (QueryServiceStatus(hs, &status) == 0) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    if (status.dwCurrentState != SERVICE_STOPPED &&
        status.dwCurrentState != SERVICE_STOP_PENDING
        ) {
        // 发送关闭服务请求
        if (ControlService(
            hs,                         // 服务句柄
            SERVICE_CONTROL_STOP,       // 控制码：通知服务应该停止
            &status                     // 接收最新的服务状态信息
        ) == 0) {
            CloseServiceHandle(hs);
            CloseServiceHandle(schSCManager);
            return FALSE;
        }

        // 判断超时
        INT timeOut = 0;
        while (status.dwCurrentState != SERVICE_STOPPED) {
            timeOut++;
            QueryServiceStatus(hs, &status);
            Sleep(50);
        }
        if (timeOut > 80) {
            CloseServiceHandle(hs);
            CloseServiceHandle(schSCManager);
            return FALSE;
        }
    }

    CloseServiceHandle(hs);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// 卸载驱动
BOOL unloadDvr(CONST WCHAR serviceName[20]) {
    // 打开服务控制管理器数据库
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                   // 目标计算机的名称,NULL：连接本地计算机上的服务控制管理器
        NULL,                   // 服务控制管理器数据库的名称，NULL：打开 SERVICES_ACTIVE_DATABASE 数据库
        SC_MANAGER_ALL_ACCESS   // 所有权限
    );
    if (schSCManager == NULL) {
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    // 打开服务
    SC_HANDLE hs = OpenService(
        schSCManager,           // 服务控件管理器数据库的句柄
        serviceName,            // 要打开的服务名
        SERVICE_ALL_ACCESS      // 服务访问权限：所有权限
    );
    if (hs == NULL) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    // 删除服务
    if (DeleteService(hs) == 0) {
        CloseServiceHandle(hs);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    CloseServiceHandle(hs);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

int main()
{
    if (installDvr(L"C:\\Test_Driver_1.sys", L"LYSM_service") == TRUE) {
        cout << "installDvr success." << endl;
    }
    if (startDvr(L"LYSM_service") == TRUE) {
        cout << "startDvr success." << endl;
    }
    if (stopDvr(L"LYSM_service") == TRUE) {
        cout << "stopDvr success." << endl;
    }
    if (unloadDvr(L"LYSM_service") == TRUE) {
        cout << "unloadDvr success." << endl;
    }
    getchar();
    return 0;
}
#endif // 0
