# Active Station

## Overview
**Active Station** is an open-source technology that allows you to create a variant of in-process COM servers that expose their objects to Windows scripts (like JScript or VBScript). COM servers built based on Active Station don't conform to the usual COM standards. In particular, they export **a single routine named `StationInject`** to do all the magic, unlike ordinary COM servers which export the four routines [`DllGetClassObject`](https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-dllgetclassobject), [`DllCanUnloadNow`](https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-dllcanunloadnow), [`DllRegisterServer`](https://learn.microsoft.com/en-us/windows/win32/api/olectl/nf-olectl-dllregisterserver), and [`DllUnregisterServer`](https://learn.microsoft.com/en-us/windows/win32/api/olectl/nf-olectl-dllunregisterserver). They have a couple of advantages over ordinary COM servers:

+ They do not require to be *registered in the system* by use of the [`regsvr32.exe` program](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/regsvr32). Interestingly, in order to instantiate them (from a script), what you need to supply is just **the name of DLL file of the COM server** rather than a *ProgID* or a *CLSID*.
+ They require less amount of source code to write. Thus, it's less time-consuming to create an Active-Station-based COM server than to create an ordinary one.
+ Their executable code has a somewhat obscure structure, making the DLL difficult to be analyzed by antivirus softwares.

## Purpose
Active Station provides an alternative way to program a COM server that exposes objects to scripts. Typically, within the implementation of the exposed objects, you make calls to the **low-level Windows API functions**. Later a script (e.g written in JScript) can access those objects and invoke their methods and properties to indirectly make use of the Windows API functions. Therefore, the ultimate purpose of Active Station is to enable script developers **to extend the capabilities of their scripts by providing indirect access to the Win32 facilities**.

## Where Applicable
Active Station can be used on **Microsoft Windows** operating systems.

## Developer Audience
Active Station is intended for **experienced C++ programmers** and **JScript/VBScript programmers** who work in either **Windows Script Host (WSH)** or **HTML Applications (HTA)** environment.

A solid understanding of **COM and ActiveX technologies** is prerequisite to the developmental use of Active Station.

## In This Section
For more information on how to use Active Station, see [Developing an Active Station Project](docs/writing-project.md).

For a sample program that uses Active Station, visit the [TestStation repository](https://github.com/javadbayat/TestStation).
