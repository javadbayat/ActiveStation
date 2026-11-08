# Writing the Server DLL
This topic describes how to **write the server DLL** using **Visual C++**. In this discussion, we consider implementing a simple **Station object** that has a single method named `MoveCursor`. Say, the client script would call that method as follows in order to move the cursor to the point at `[600, 200]` on the screen:

```js
station.MoveCursor(600, 200);
```

## Step 1: Writing the COM Class
First of all we need to define a COM class representing the Station object. This COM class can have any name of your choice, and it should inherit from [the IDispatch interface](https://learn.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-idispatch). In this example, it defines one additional method, named `MoveCursor`, used to move the cursor to a given point on the screen. Supporting **IDispatch** is essential to expose our `MoveCursor` method to the client script.

So the definition of our COM class will look like the following:

```cpp
#define DISPID_STATION_MoveCursor (1)

class CStationService : public IDispatch
{
private:
    ULONG m_cRef;
public:
    CStationService(void);
    ~CStationService(void);

    // IUnknown Members
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    // IDispatch Members
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT FAR* pctInfo);
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo);
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgDispId);
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, UINT FAR* puArgErr);

    // StationService Members
    void STDMETHODCALLTYPE MoveCursor(int X, int Y);
};
```

Now we have to implement our COM class. The implementation of `MoveCursor` method simply calls [the Win32 function `SetCursorPos`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcursorpos) with the given coordinate values. If `SetCursorPos` fails, it will call [`GetLastError`](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror), convert the last error code to an [*HRESULT*](https://learn.microsoft.com/en-us/windows/win32/com/structure-of-com-error-codes) value, and throw a [`_com_error` exception](https://learn.microsoft.com/en-us/cpp/cpp/com-error-class?view=msvc-170) with the *HRESULT* obtained.

```cpp
void STDMETHODCALLTYPE CStationService::MoveCursor(int X, int Y)
{
    if (!SetCursorPos( X, Y ))
        _com_issue_error( HRESULT_FROM_WIN32( GetLastError() ) );
}
```

Here we do not cover how to implement the [IDispatch](https://learn.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-idispatch) and [IUnknown](https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown) interfaces. For details, refer to **the Win32 documentation** and [this Stack Overflow post](https://stackoverflow.com/q/3747414/8492420). Anyway, in this example, our implementation of **IUnknown** and **IDispatch** is the following:

```cpp
// IUnknown Members

STDMETHODIMP CStationService::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    if (ppv)
    {
        *ppv = NULL;

        if (IsEqualIID( riid, IID_IDispatch ) || IsEqualIID( riid, IID_IUnknown ))
            *ppv = (IDispatch *) (this);
        else
            hr = E_NOINTERFACE;

        if (*ppv)
            AddRef();
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

STDMETHODIMP_(ULONG) CStationService::AddRef(void)
{
    return((ULONG) (InterlockedIncrement((long *) &m_cRef)));
}

STDMETHODIMP_(ULONG) CStationService::Release(void)
{
    ULONG cRef = (ULONG) (InterlockedDecrement((long *) &m_cRef));
    if (!cRef)
        delete this;

    return cRef;
}

// IDispatch Members

STDMETHODIMP CStationService::GetTypeInfoCount(UINT FAR* pctInfo)
{
    if (!pctInfo)
        return E_INVALIDARG;

    *pctInfo = 0;
    return S_OK;
}

STDMETHODIMP CStationService::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStationService::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgDispId)
{
    HRESULT hr = S_OK;

    for (UINT i = 0; i < cNames; i++)
    {
        if (!_wcsicmp( rgszNames[i], L"MoveCursor" ))
            rgDispId[i] = DISPID_STATION_MoveCursor;
        else
        {
            rgDispId[i] = DISPID_UNKNOWN;
            hr = DISP_E_UNKNOWNNAME;
        }
    }

    return hr;
}

STDMETHODIMP CStationService::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, UINT FAR* puArgErr)
{
    try
    {
        if (!pDispParams)
            _com_issue_error( E_INVALIDARG );

        if (!IsEqualIID( riid, IID_NULL ))
            _com_issue_error( DISP_E_UNKNOWNINTERFACE );

        HRESULT hr = S_OK;
        _variant_t varTempResult;

        switch (dispIdMember)
        {
        case DISPID_STATION_MoveCursor :
            {
                if (!(wFlags & DISPATCH_METHOD))
                    _com_issue_error( DISP_E_MEMBERNOTFOUND );

                if (pDispParams->cArgs != 2)
                    _com_issue_error( DISP_E_BADPARAMCOUNT );

                _variant_t X( pDispParams->rgvarg + 1 );
                _variant_t Y( pDispParams->rgvarg );
                MoveCursor( X, Y );

                break;
            }
        default:
            {
                _com_issue_error( DISP_E_MEMBERNOTFOUND );
                break;
            }
        }

        if (pVarResult)
        {
            hr = VariantCopy( pVarResult, &varTempResult );
            if (FAILED(hr))
                _com_issue_error( hr );
        }
    }
    catch (_com_error &ce)
    {
        return ce.Error();
    }

    return S_OK;
}
```

> [!NOTE]
> You can find the full source code of the sample DLL presented in this article in [the TestStation repository](https://github.com/javadbayat/TestStation).

## Step 2: Writing the `StationGetObject` Routine
There is a function, `StationGetObject`, that should be implemented in DLLs that support Active Station. This function serves as the interface between your DLL implementation and the Active Station static library.

In your code, this function must be defined (in global scope) with the name `StationGetObject` and the following signature:

```cpp
STDAPI StationGetObject(LPUNKNOWN *ppStation)
```

> [!NOTE]
> If the compiler finds no function named `StationGetObject` with exactly the above signature, then your DLL won't compile. So be careful to correctly write the definition of this function and its parameters.

Your implementation of `StationGetObject` typically creates an instance of the COM class you wrote in **Step 1**, and then returns the newly-created object in the `ppStation` argument. If an instance of the COM class was successfully created, the function should return `S_OK`. If instance creation failed, for example, due to a lack of memory, the function should return an appropriate error code, such as `E_OUTOFMEMORY`.

The following is an example of an implementation of `StationGetObject`:

```cpp
STDAPI StationGetObject(LPUNKNOWN *ppStation)
{
    try
    {
        (*ppStation = new CStationService)->AddRef();
    }
    catch (std::bad_alloc &)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
```

Note that in cases where `StationGetObject` returns a failure code (e.g. `E_OUTOFMEMORY`), the client script will fail to instantiate your DLL, generating a run-time error.

> [!NOTE]
> Please be aware that the execution of the `StationGetObject` function should not take longer than 3 seconds. If the duration of its execution exceeds 3 seconds, then the client script will abort the instantiation of your DLL and issue a run-time error. So remember not to perform lengthy operations in your implementation of `StationGetObject`.

## Step 3: Importing the Static Library and Writing `DllMain`
Copy the Active Station static library into your Visual Studio project folder, that is, copy the two files `libstation.x86.lib` and `libstation.x64.lib` that you obtained by compiling the library's source code that is provided in this repository.

Next, import the Active Station static library into your DLL by adding these [*pragma* directives](https://learn.microsoft.com/en-us/cpp/preprocessor/pragma-directives-and-the-pragma-keyword?view=msvc-170) to your code:

```cpp
#ifdef _WIN64
    #pragma comment(lib, "libstation.x64.lib")
#else
    #pragma comment(lib, "libstation.x86.lib")
#endif
```

To complete your DLL code, you must also include a [`DllMain` function](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain). In Active-Station-based DLLs, the `DllMain` function typically does nothing more than simply returning `TRUE`, as shown in the following example:

```cpp
BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{
    return TRUE;
}
```

## Step 4: Writing a Module Definition File
The last step you should take before compiling your DLL is add a short [Module Definition File (`.def` file)](https://learn.microsoft.com/en-us/cpp/build/reference/module-definition-dot-def-files?view=msvc-170) to your Visual Studio project, which must contain the following code:

```
LIBRARY XXXXX
EXPORTS
    StationInject
```

where `XXXXX` is **the name of your Visual Studio project**.

As can be seen in the preceding code, that Module Definition file exports a routine named `StationInject` from your DLL. For sure, you do not need to care about what that routine does or where it is defined. The `StationInject` routine is actually defined within the static library, and is used internally to load your DLL into the process of your client script (e.g. `wscript.exe` or `mshta.exe` process).

## Step 5: Compile your DLL
At this point, your DLL is ready to be compiled! Feel free to build your DLL for **both x86 and x64 target architectures**. Build it in **Release mode**.
