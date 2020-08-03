/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "connectivitymonitor.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <atlbase.h>
#include <netlistmgr.h>

class NetworkEventHandler : public INetworkListManagerEvents
{
public:
    NetworkEventHandler()
        : m_lRefCnt(1){};
    virtual ~NetworkEventHandler(){};

    HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void **ppvObject)
    {
        HRESULT hr = S_OK;
        if (IsEqualIID(riid, IID_IUnknown)) {
            *ppvObject = (IUnknown *) this;
        } else if (IsEqualIID(riid, IID_INetworkListManagerEvents)) {
            *ppvObject = (INetworkListManagerEvents *) this;
        } else {
            hr = E_NOINTERFACE;
        }

        return hr;
    };
    ULONG STDMETHODCALLTYPE
    AddRef()
    {
        return (ULONG) InterlockedIncrement(&m_lRefCnt);
    };
    ULONG STDMETHODCALLTYPE
    Release()
    {
        LONG res = InterlockedDecrement(&m_lRefCnt);
        if (res == 0) {
            delete this;
        }
        return (ULONG) res;
    };

    virtual HRESULT STDMETHODCALLTYPE
    ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
    {
        qDebug() << "connectivity changed: " << newConnectivity;
        if (connectivityChangedCb_) {
            connectivityChangedCb_();
        }
        return S_OK;
    };

    void
    setOnConnectivityChangedCallBack(std::function<void()> &&cb)
    {
        connectivityChangedCb_ = cb;
    };

private:
    LONG m_lRefCnt;

    std::function<void()> connectivityChangedCb_;
};

ConnectivityMonitor::ConnectivityMonitor(QObject *parent)
    : QObject(parent)
{
    CoInitialize(NULL);

    IUnknown *pUnknown = NULL;

    HRESULT hr = CoCreateInstance(CLSID_NetworkListManager,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IUnknown,
                                  (void **) &pUnknown);
    if (FAILED(hr)) {
        return;
    }

    pNetworkListManager_ = NULL;
    hr = pUnknown->QueryInterface(IID_INetworkListManager, (void **) &pNetworkListManager_);
    if (FAILED(hr)) {
        destroy();
        pUnknown->Release();
        return;
    }

    pCPContainer_ = NULL;
    hr = pNetworkListManager_->QueryInterface(IID_IConnectionPointContainer,
                                              (void **) &pCPContainer_);
    if (FAILED(hr)) {
        destroy();
        pUnknown->Release();
        return;
    }

    pConnectPoint_ = NULL;
    hr = pCPContainer_->FindConnectionPoint(IID_INetworkListManagerEvents, &pConnectPoint_);
    if (SUCCEEDED(hr)) {
        cookie_ = NULL;
        netEventHandler_ = new NetworkEventHandler;
        netEventHandler_->setOnConnectivityChangedCallBack([this] { emit connectivityChanged(); });
        hr = pConnectPoint_->Advise((IUnknown *) netEventHandler_, &cookie_);
    } else {
        destroy();
    }

    pUnknown->Release();
}

bool
ConnectivityMonitor::isOnline()
{
    if (!pNetworkListManager_) {
        return false;
    }
    VARIANT_BOOL IsConnect = VARIANT_FALSE;
    HRESULT hr = pNetworkListManager_->get_IsConnectedToInternet(&IsConnect);
    if (SUCCEEDED(hr)) {
        return IsConnect == VARIANT_TRUE;
    }
    return false;
}

void
ConnectivityMonitor::destroy()
{
    if (pConnectPoint_) {
        pConnectPoint_->Unadvise(cookie_);
        pConnectPoint_->Release();
    }
    if (pCPContainer_) {
        pCPContainer_->Release();
    }
    if (pNetworkListManager_) {
        pNetworkListManager_->Release();
        pNetworkListManager_ = NULL;
    }
}

ConnectivityMonitor::~ConnectivityMonitor()
{
    destroy();
    CoUninitialize();
}
#endif // Q_OS_WIN