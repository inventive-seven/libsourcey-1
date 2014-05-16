//
// LibSourcey
// Copyright (C) 2005, Sourcey <http://sourcey.com>
//
// LibSourcey is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// LibSourcey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// This file uses functions from POCO C++ Libraries (license below)
//

#include "scy/net/sslmanager.h"
#include "scy/net/sslcontext.h"
#include "scy/singleton.h"


using namespace std;


namespace scy {
namespace net {


SSLManager::SSLManager()
{
}


SSLManager::~SSLManager()
{
	shutdown();
}


void SSLManager::shutdown()
{
	PrivateKeyPassphraseRequired.clear();
	ClientVerificationError.clear();
	ServerVerificationError.clear();
	_defaultServerContext = nullptr;
	_defaultClientContext = nullptr;
}


/*
namespace
{
	static Singleton<SSLManager> singleton;
}
*/

Singleton<SSLManager>& singleton() 
{
	static Singleton<SSLManager> singleton;
	return singleton;
}


SSLManager& SSLManager::instance()
{
	return *singleton().get();
}


void SSLManager::destroy()
{
	singleton().destroy();
}


void SSLManager::initializeServer(SSLContext::Ptr ptrContext)
{
	_defaultServerContext = ptrContext;
}


void SSLManager::initializeClient(SSLContext::Ptr ptrContext)
{
	_defaultClientContext = ptrContext;
}


SSLContext::Ptr SSLManager::defaultServerContext()
{
	Mutex::ScopedLock lock(_mutex);
	return _defaultServerContext;
}


SSLContext::Ptr SSLManager::defaultClientContext()
{
	Mutex::ScopedLock lock(_mutex);
	return _defaultClientContext;
}


int SSLManager::verifyCallback(bool server, int ok, X509_STORE_CTX* pStore)
{
	if (!ok) {
		X509* pCert = X509_STORE_CTX_get_current_cert(pStore);
		crypto::X509Certificate x509(pCert, true);
		int depth = X509_STORE_CTX_get_error_depth(pStore);
		int err = X509_STORE_CTX_get_error(pStore);
		std::string error(X509_verify_cert_error_string(err));
		VerificationErrorDetails args(x509, depth, err, error);
		if (server)
			SSLManager::instance().ServerVerificationError.emit(&SSLManager::instance(), args);
		else
			SSLManager::instance().ClientVerificationError.emit(&SSLManager::instance(), args);
		ok = args.getIgnoreError() ? 1 : 0;
	}

	return ok;
}


int SSLManager::privateKeyPassphraseCallback(char* pBuf, int size, int flag, void* userData)
{
	std::string pwd;
	SSLManager::instance().PrivateKeyPassphraseRequired.emit(&SSLManager::instance(), pwd);

	strncpy(pBuf, (char *)(pwd.c_str()), size);
	pBuf[size - 1] = '\0';
	if (size > (int)pwd.length())
		size = (int)pwd.length();

	return size;
}


void initializeSSL()
{
	crypto::initializeEngine();
}


void uninitializeSSL()
{
	SSLManager::instance().shutdown();
	crypto::uninitializeEngine();
}


//
// Verification Error Details
//


VerificationErrorDetails::VerificationErrorDetails(const crypto::X509Certificate& cert, int errDepth, int errNum, const std::string& errMsg):
	_cert(cert),
	_errorDepth(errDepth),
	_errorNumber(errNum),
	_errorMessage(errMsg),
	_ignoreError(false)
{
}


VerificationErrorDetails::~VerificationErrorDetails()
{
}



} } // namespace scy::net


// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
