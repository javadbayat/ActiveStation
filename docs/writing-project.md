# Developing an Active Station Project
An Active Station project consists of **a server DLL** and **a client script**. The client script loads the server DLL and gets a reference to the primary COM object provided by the DLL, known as **the Station object**. Then the script invokes the methods and properties of the Station object to perform special operations. That DLL acts as **the COM Server** and implements the Station object; that script is **the COM Client**.

The Active Station technology features **a C++ static library** that you have to import into your server DLLs. The complete source code of this static library has been published **in this repository**. Before getting started with Active Station, you must obtain its source code from the repository, then build it using **Microsoft Visual Studio** once for **x86** and once for **x64 targets** (in **Release mode**). Upon successful build, you will get two static library files, namely `libstation.x86.lib` and `libstation.x64.lib`. You will be able to use these files in every Active Station project of your own.

## In this section
For more information, see the following topics:

+ [Writing the Server DLL](writing-server-dll.md)
+ [Writing the Client Script](writing-client-script.md)
