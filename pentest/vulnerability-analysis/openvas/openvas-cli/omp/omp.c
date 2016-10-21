/* OMP Command Line Interface
 * $Id$
 * Description: A command line client for the OpenVAS Management Protocol
 *
 * Authors:
 * Matthew Mundell <matthew.mundell@greenbone.net>
 * Michael Wiegand <michael.wiegand@greenbone.net>
 *
 * Copyright:
 * Copyright (C) 2009, 2010, 2015, 2016 Greenbone Networks GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file  omp.c
 * @brief The OMP Command Line Interface
 *
 * This command line tool provides command line arguments
 * corresponding to the OMP protocol commands as well as a
 * direct method to send OMP protocol commands (which is
 * based on XML).
 */

/**
 * \mainpage
 * \section Introduction
 * \verbinclude README
 *
 * \section Installation
 * \verbinclude INSTALL
 *
 * \section copying License Information
 * \verbinclude COPYING
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <termios.h>  /* for tcsetattr */
#endif
#include <unistd.h>   /* for getpid */

#include <openvas/misc/openvas_server.h>
#include <openvas/base/openvas_file.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#ifndef _WIN32
#include <openvas/misc/openvas_logging.h>
#endif
#include <openvas/omp/omp.h>
#include <openvas/omp/xml.h>

/**
 * @brief The name of this program.
 */
#define OMP_PROGNAME "omp"

/**
 * @brief Default Manager (openvasmd) address.
 */
#define OPENVASMD_ADDRESS "127.0.0.1"

/**
 * @brief Default Manager port.
 */
#define OPENVASMD_PORT 9390


/**
 * @brief Substring to replace when using --send-file option.
 */
#define SENDFILE_STR "SENDFILE"

/**
 * @brief Default timeout value for OMP pings.
 */
#define DEFAULT_PING_TIMEOUT 10

/* Connection handling. */

/**
 * @brief Information needed to handle a connection to a server.
 */
typedef struct
{
  gnutls_session_t session;     ///< GnuTLS Session to use.
  int socket;                   ///< Socket to server.
  gchar *username;              ///< Username with which to connect.
  gchar *password;              ///< Password for user with which to connect.
  gchar *host_string;           ///< Server host string.
  gchar *port_string;           ///< Server port string.
  gint port;                    ///< Port of server.
  gboolean use_certs;           ///< Use client certificates to authenticate.
  gchar *client_cert_path;      ///< The file with the client certification
  gchar *client_key_path;       ///< The file with the client key
  gchar *client_ca_cert_path;   ///< The file with the client ca certification
  gint timeout;                 ///< Timeout of request.
} server_connection_t;

/**
 * @brief Read connection parameters from a key-file.
 *
 * If the key-file could not be loaded, emit warning and return g_malloc0'ed
 * struct. If keys are missing, set the corresponding fields in the struct to
 * 0.
 *
 * @param  conf_file_path  Path to key-file.
 *
 * @return Struct containing the parameters read from key-file (port, host,
 *         user, password).
 */
static server_connection_t *
connection_from_file (const gchar * conf_file_path)
{
  assert (conf_file_path);
  GKeyFile *key_file = g_key_file_new ();
  GError *error = NULL;
  server_connection_t *connection = g_malloc0 (sizeof (*connection));

  /* Load key file. */
  if (g_key_file_load_from_file (key_file, conf_file_path, 0, &error) == FALSE)
    {
      /* Be chatty about non trivial error (file does exist). */
      if (g_file_test (conf_file_path, G_FILE_TEST_EXISTS))
        g_warning ("Could not load connection configuration from %s: %s",
                   conf_file_path, error->message);

      g_error_free (error);
      g_key_file_free (key_file);
      return connection;
    }

#if 0
  /* Check for completeness. */
  if (g_key_file_has_key (key_file, "Connection", "host", &error) == FALSE
      || g_key_file_has_key (key_file, "Connection", "port", &error) == FALSE
      || g_key_file_has_key (key_file, "Connection", "username",
                             &error) == FALSE
      || g_key_file_has_key (key_file, "Connection", "password",
                             &error) == FALSE)
    {
      g_warning ("Connection configuration file misses entrie(s): %s",
                 error->message);
      g_error_free (error);
      return NULL;
    }
#endif

  /* Fill struct if any values found. */
  connection->host_string =
    g_key_file_get_string (key_file, "Connection", "host", NULL);
  connection->port_string =
    g_key_file_get_string (key_file, "Connection", "port", NULL);
  connection->username =
    g_key_file_get_string (key_file, "Connection", "username", NULL);
  connection->password =
    g_key_file_get_string (key_file, "Connection", "password", NULL);

  g_key_file_free (key_file);

  return connection;
}

/**
 * @brief Connect to an openvas-manager.
 *
 * Prints error on failure.
 *
 * @return 0 on success, -1 on failure.
 */
static int
manager_open (server_connection_t * connection)
{
  if (connection->use_certs)
    {
      char *ca_pub = NULL, *key_pub = NULL, *key_priv = NULL;
      gsize len = 0;
      GError *error = NULL;

      if (!g_file_get_contents (connection->client_ca_cert_path, &ca_pub, &len, &error))
        {
          fprintf (stderr, "%s: %s\n", connection->client_ca_cert_path, error->message);
          g_error_free (error);
          return -1;
        }
      if (!g_file_get_contents (connection->client_cert_path, &key_pub, &len, &error))
       {
         fprintf (stderr, "%s: %s\n", connection->client_cert_path, error->message);
         g_error_free (error);
         g_free (ca_pub);
         return -1;
       }
      if (!g_file_get_contents (connection->client_key_path, &key_priv, &len, &error))
       {
         fprintf (stderr, "%s: %s\n", connection->client_key_path, error->message);
         g_error_free (error);
         g_free (ca_pub);
         g_free (key_pub);
         return -1;
       }
      connection->socket = openvas_server_open_with_cert
                            (&connection->session, connection->host_string,
                             connection->port, ca_pub, key_pub, key_priv);
      g_free (ca_pub);
      g_free (key_pub);
      g_free (key_priv);
    }
  else
    connection->socket = openvas_server_open
                          (&connection->session, connection->host_string,
                           connection->port);

  if (connection->socket == -1)
    {
      fprintf (stderr, "Failed to acquire socket.\n");
      return -1;
    }

  if (connection->use_certs)
    return 0;

  if (connection->username && connection->password)
    {
      if (omp_authenticate
          (&connection->session, connection->username, connection->password))
        {
          openvas_server_close (connection->socket, connection->session);
          fprintf (stderr, "Failed to authenticate.\n");
          return -1;
        }
    }

  return 0;
}

/**
 * @brief Closes the connection to a manager.
 *
 * @return 0 on success, -1 on failure.
 */
static int
manager_close (server_connection_t * server)
{
  return openvas_server_close (server->socket, server->session);
}

/**
 * @brief Print tasks.
 *
 * @param[in]  tasks  Tasks.
 *
 * @return 0 success, -1 error.
 */
static int
print_tasks (entities_t tasks)
{
  entity_t task;
  while ((task = first_entity (tasks)))
    {
      if (strcmp (entity_name (task), "task") == 0)
        {
          entity_t entity, report;
          entities_t reports;
          const char *id, *name, *status, *progress;

          id = entity_attribute (task, "id");
          if (id == NULL)
            {
              fprintf (stderr, "Failed to parse task ID.\n");
              return -1;
            }

          entity = entity_child (task, "name");
          if (entity == NULL)
            {
              fprintf (stderr, "Failed to parse task name.\n");
              return -1;
            }
          name = entity_text (entity);

          entity = entity_child (task, "status");
          if (entity == NULL)
            {
              fprintf (stderr, "Failed to parse task status.\n");
              return -1;
            }
          status = entity_text (entity);

          entity = entity_child (task, "progress");
          if (entity == NULL)
            {
              fprintf (stderr, "Failed to parse task progress.\n");
              return -1;
            }
          progress = entity_text (entity);

          printf ("%s  %-7s", id, status);
          if (strcmp (status, "Running") == 0)
            printf (" %2s%%  %s\n", progress, name);
          else
            printf ("      %s\n", name);

          /* Print any reports indented under the task. */

          entity = entity_child (task, "reports");
          if (entity == NULL)
            {
              tasks = next_entities (tasks);
              continue;
            }

          reports = entity->entities;
          while ((report = first_entity (reports)))
            {
              if (strcmp (entity_name (report), "report") == 0)
                {
                  entity_t result_count;
                  const char *id, *status, *holes, *infos, *logs, *warnings;
                  const char *time_stamp;

                  id = entity_attribute (report, "id");
                  if (id == NULL)
                    {
                      fprintf (stderr, "Failed to parse report ID.\n");
                      return -1;
                    }

                  entity = entity_child (report, "scan_run_status");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report status.\n");
                      return -1;
                    }
                  status = entity_text (entity);

                  result_count = entity_child (report, "result_count");
                  if (result_count == NULL)
                    {
                      fprintf (stderr, "Failed to parse report result_count.\n");
                      return -1;
                    }

                  entity = entity_child (result_count, "hole");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report hole.\n");
                      return -1;
                    }
                  holes = entity_text (entity);

                  entity = entity_child (result_count, "info");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report info.\n");
                      return -1;
                    }
                  infos = entity_text (entity);

                  entity = entity_child (result_count, "log");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report log.\n");
                      return -1;
                    }
                  logs = entity_text (entity);

                  entity = entity_child (result_count, "warning");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report warning.\n");
                      return -1;
                    }
                  warnings = entity_text (entity);

                  entity = entity_child (report, "timestamp");
                  if (entity == NULL)
                    {
                      fprintf (stderr, "Failed to parse report timestamp.\n");
                      return -1;
                    }
                  time_stamp = entity_text (entity);

                  printf ("  %s  %-7s  %2s  %2s  %2s  %2s  %s\n", id, status,
                          holes, warnings, infos, logs, time_stamp);
                }
              reports = next_entities (reports);
            }
        }
      tasks = next_entities (tasks);
    }
  return 0;
}

/**
 * @brief Print configs.
 *
 * @param[in]  configs  Configs.
 *
 * @return 0 success, -1 error.
 */
static int
print_configs (entities_t configs)
{
  entity_t config;
  while ((config = first_entity (configs)))
    {
      if (strcmp (entity_name (config), "config") == 0)
        {
          entity_t entity;
          const char *id, *name;

          id = entity_attribute (config, "id");
          if (id == NULL)
            {
              fprintf (stderr, "Failed to parse config ID.\n");
              return -1;
            }

          entity = entity_child (config, "name");
          if (entity == NULL)
            {
              fprintf (stderr, "Failed to parse config name.\n");
              return -1;
            }
          name = entity_text (entity);

          printf ("%s  %s\n", id, name);
        }
      configs = next_entities (configs);
    }
  return 0;
}

/**
 * @brief Print targets.
 *
 * @param[in]  targets  Targets.
 *
 * @return 0 success, -1 error.
 */
static int
print_targets (entities_t targets)
{
  entity_t target;
  while ((target = first_entity (targets)))
    {
      if (strcmp (entity_name (target), "target") == 0)
        {
          entity_t entity;
          const char *id, *name;

          id = entity_attribute (target, "id");
          if (id == NULL)
            {
              fprintf (stderr, "Failed to parse target ID.\n");
              return -1;
            }

          entity = entity_child (target, "name");
          if (entity == NULL)
            {
              fprintf (stderr, "Failed to parse target name.\n");
              return -1;
            }
          name = entity_text (entity);

          printf ("%s  %s\n", id, name);
        }
      targets = next_entities (targets);
    }
  return 0;
}

/**
 * @brief Get the list of scan configs.
 *
 * @param[in]  session         Pointer to GNUTLS session.
 * @param[out] status          Status return.  On success contains GET_CONFIGS
 *                             response.
 *
 * @return 0 on success, -1 or OMP response code on error.
 */
int
get_configs (gnutls_session_t* session, entity_t* status)
{
  const char* status_code;
  int ret;

  if (openvas_server_sendf (session, "<get_configs/>") == -1)
    return -1;

  /* Read the response. */

  *status = NULL;
  if (read_entity (session, status)) return -1;

  /* Check the response. */

  status_code = entity_attribute (*status, "status");
  if (status_code == NULL)
    {
      free_entity (*status);
      return -1;
    }
  if (strlen (status_code) == 0)
    {
      free_entity (*status);
      return -1;
    }
  if (status_code[0] == '2') return 0;
  ret = (int) strtol (status_code, NULL, 10);
  free_entity (*status);
  if (errno == ERANGE) return -1;
  return ret;
}



/* Commands. */

/**
 * @brief Performs the OMP get_version command.
 *
 * @param  connection  Connection to manager to use.
 * @param[out] version_str Pointer to the version string.
 *
 * @return 0 success, -1 error.
 */
static int
manager_get_omp_version (server_connection_t * connection, gchar ** version_str)
{
  entity_t entity, version;

  if (openvas_server_sendf (&(connection->session), "<get_version/>")
      == -1)
    {
      manager_close (connection);
      return -1;
    }

  /* Read the response. */

  entity = NULL;
  if (read_entity (&(connection->session), &entity))
    {
      fprintf (stderr, "Failed to read response.\n");
      manager_close (connection);
      return -1;
    }

  version = entity_child (entity, "version");
  if (version == NULL)
    {
      free_entity (entity);
      fprintf (stderr, "Failed to parse version.\n");
      manager_close (connection);
      return -1;
    }

  *version_str = g_strdup (entity_text(version));

  free_entity (entity);

  return 0;
}

/**
 * @brief Performs the omp get_report command.
 *
 * @param  connection  Connection to manager to use.
 * @param  report_ids  Pointer to task_uuid id.
 * @param  format      Queried report format.
 *
 * @todo This function currently does not use library functions for getting
 * reports to ensure it works with both OMP 1.0 and 2.0. Once OMP 1.0 is
 * retired, this function should use the existing library functions.
 *
 * @return 0 success, -1 error.
 */
static int
manager_get_reports (server_connection_t * connection, gchar ** report_ids,
                     gchar * format, gchar * filter)
{
  gchar *version = NULL;
  gchar *default_format = NULL;
  gchar *format_req_str = NULL;

  if (manager_get_omp_version (connection, &version))
    {
      fprintf (stderr, "Failed to determine OMP version.\n");
      manager_close (connection);
      return -1;
    }

  if (strcmp (version, "1.0") == 0)
    {
      default_format = "XML";
      format_req_str = "format";
    }
  else if (strcmp (version, "2.0") == 0)
    {
      default_format = "d5da9f67-8551-4e51-807b-b6a873d70e34";
      format_req_str = "format_id";
    }
  else
    {
      default_format = "a994b278-1f62-11e1-96ac-406186ea4fc5";
      format_req_str = "format_id";
    }

  g_free (version);

  if (format == NULL || strcasecmp (format, default_format) == 0)
    {
      gchar *quoted_filter;
      entity_t entity, report_xml;

      quoted_filter = filter ? g_strescape (filter, "") : NULL;

      if (openvas_server_sendf (&(connection->session),
                                "<get_reports"
                                " result_hosts_only=\"0\""
                                " first_result=\"0\""
                                " sort_field=\"ROWID\""
                                " sort_order=\"1\""
                                " %s=\"%s\""
                                "%s%s%s"
                                " report_id=\"%s\"/>",
                                format_req_str,
                                format ? format :
                                default_format,
                                quoted_filter ? " filter=\"" : "",
                                quoted_filter ? quoted_filter : "",
                                quoted_filter ? "\"" : "",
                                *report_ids))
        {
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          g_free (quoted_filter);
          return -1;
        }
      g_free (quoted_filter);

      if (read_entity (&connection->session, &entity)) {
        fprintf (stderr, "Failed to get report.\n");
        manager_close (connection);
        return -1;
      }

      report_xml = entity_child (entity, "report");
      if (report_xml == NULL)
        {
          free_entity (entity);
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }

      print_entity (stdout, report_xml);
    }
  else
    {
      gchar *quoted_filter;
      guchar *report = NULL;
      gsize report_size = 0;
      char first;
      const char* status;
      entity_t entity;

      quoted_filter = filter ? g_markup_escape_text (filter, -1) : NULL;

      if (openvas_server_sendf (&(connection->session),
                                "<get_reports %s=\"%s\" report_id=\"%s\" %s%s%s/>",
                                format_req_str,
                                format,
                                *report_ids,
                                quoted_filter ? " filter=\"" : "",
                                quoted_filter ? quoted_filter : "",
                                quoted_filter ? "\"" : ""))
        {
          g_free (quoted_filter);
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }
      g_free (quoted_filter);

      /* Read the response. */

      entity = NULL;
      if (read_entity (&connection->session, &entity))
        {
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }

      /* Check the response. */

      status = entity_attribute (entity, "status");
      if (status == NULL)
        {
          free_entity (entity);
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }
      if (strlen (status) == 0)
        {
          free_entity (entity);
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }
      first = status[0];
      if (first == '2')
        {
          const char* report_64;
          entity_t report_xml;

          report_xml = entity_child (entity, "report");
          if (report_xml == NULL)
            {
              free_entity (entity);
              fprintf (stderr, "Failed to get report.\n");
              manager_close (connection);
              return -1;
            }

          report_64 = entity_text (report_xml);
          if (strlen (report_64) == 0)
            {
              report = (guchar *) g_strdup ("");
              report_size = 0;
            }
          else
            {
              report = g_base64_decode (report_64, &report_size);
            }

          free_entity (entity);
        }
      else
        {
          free_entity (entity);
          fprintf (stderr, "Failed to get report.\n");
          manager_close (connection);
          return -1;
        }

      if (fwrite (report, 1, report_size, stdout) < report_size)
        {
          fprintf (stderr, "Failed to write entire report.\n");
          manager_close (connection);
          return -1;
        }
      g_free (report);
    }

  return 0;
}

/**
 * @brief Performs the OMP get_report_formats command.
 *
 * @param  connection  Connection to manager to use.
 *
 * @return 0 success, -1 error.
 */
static int
manager_get_report_formats (server_connection_t * connection)
{
  entity_t entity, format;
  entities_t formats;

  if (openvas_server_sendf (&(connection->session), "<get_report_formats/>")
      == -1)
    {
      manager_close (connection);
      return -1;
    }

  /* Read the response. */

  entity = NULL;
  if (read_entity (&(connection->session), &entity))
    {
      fprintf (stderr, "Failed to read response.\n");
      manager_close (connection);
      return -1;
    }

  formats = entity->entities;
  while ((format = first_entity (formats)))
    {
      if (strcmp (entity_name (format), "report_format") == 0)
        {
          const char *id;
          entity_t name;

          id = entity_attribute (format, "id");
          if (id == NULL)
            {
              free_entity (entity);
              fprintf (stderr, "Failed to parse report format ID.\n");
              manager_close (connection);
              return -1;
            }

          name = entity_child (format, "name");
          if (name == NULL)
            {
              free_entity (entity);
              fprintf (stderr, "Failed to parse report format name.\n");
              manager_close (connection);
              return -1;
            }

          printf ("%s  %s\n", id, entity_text (name));
        }
      formats = next_entities (formats);
    }

  free_entity (entity);

  return 0;
}

/**
 * @brief Reads an entire line from a stream, suppressing character output.
 *
 * @param[out]  lineptr  Location of the buffer where the line is stored.
 * @param[out]  n  Size of allocated buffer in lineptr is not null.
 * @param[in] stream  Stream from which the line should be read.
 *
 * This function mimics the behaviour of getline (). Please see the man page of
 * getline () for additional information about the parameters. This function was
 * taken from the example provided in the GNU C Library, for example at
 * http://www.gnu.org/s/libc/manual/html_node/getpass.html.
 *
 * @todo Move this function to openvas-libraries since openvas-administrator
 * uses it as well.
 */
#ifndef _WIN32
ssize_t
read_password (char **lineptr, size_t *n, FILE *stream)
{
  struct termios old, new;
  int nread;

  /* Turn echoing off and fail if we can't. */
  if (tcgetattr (fileno (stream), &old) != 0)
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0)
    return -1;

  /* Read the password. */
  nread = getline (lineptr, n, stream);

  /* Restore terminal. */
  (void) tcsetattr (fileno (stream), TCSAFLUSH, &old);

  return nread;
}
#else
ssize_t
read_password (char **lineptr, size_t *n, FILE *stream)
{
  HANDLE hConsoleHandle;
  DWORD nread;
  (void) stream;
  unsigned int bufSize = 512;

  if (!lineptr || !n)
    {
      return -1;
    }

  bufSize = *n ? *n : bufSize;

  hConsoleHandle = CreateFile ("CONIN$", GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                               NULL);

  if (hConsoleHandle == INVALID_HANDLE_VALUE)
    return -1;

  if (!SetConsoleMode (hConsoleHandle, ENABLE_LINE_INPUT |
                       ENABLE_PROCESSED_INPUT))
    return -1;

  /* Read the password */
  {
    wchar_t wcsbuf[bufSize];
    int n2;
    ReadConsoleW (hConsoleHandle, &wcsbuf, bufSize - 2, &nread, NULL);
    CloseHandle (hConsoleHandle);

    if (nread <= 2)
      return -1;

    /* Remove CR/LF */
    wcsbuf[nread - 2] = '\n';
    wcsbuf[nread - 1] = '\0';

    /* Convert to UTF-8 */
    n2 = WideCharToMultiByte (CP_UTF8, 0, wcsbuf, nread - 1,
                              NULL, 0, NULL, NULL);

    if (n2 < 0)
      return -1;

    *lineptr = g_malloc (n2 + 1);

    nread = WideCharToMultiByte (CP_UTF8, 0, wcsbuf, nread - 1,
                                 *lineptr, n2, NULL, NULL);
    if (nread < 0)
      {
        g_free (*lineptr);
        return -1;
      }
  }

  return nread;
}
#endif


/**
 * @brief GNUTLS log handler
 */
static void
my_gnutls_log_func (int level, const char *text)
{
  fprintf (stderr, "[%d] (%d) %s", getpid (), level, text);
  if (*text && text[strlen (text) -1] != '\n')
    putc ('\n', stderr);
}

static int
replace_send_file_xml (char **xml, const char *path)
{
  char *s, *content;
  if (!xml || !*xml || !path)
    return 1;

  s = strstr (*xml, SENDFILE_STR);
  if (!s)
    {
      fprintf (stderr, "%s not found in xml command.\n", SENDFILE_STR);
      return 1;
    }
  content = openvas_file_as_base64 (path);
  if (!content)
    return 1;
  *s = '\0';
  s += strlen (SENDFILE_STR);
  *xml = g_strdup_printf ("%s%s%s", *xml, content, s);
  g_free (content);
  return 0;
}

/* Entry point. */

int
main (int argc, char **argv)
{
  server_connection_t *connection = NULL;
  /* The return status of the command. */
  int exit_status = -1;

  /* Global options. */
  static gboolean prompt = FALSE;
  static gboolean print_version = FALSE;
  static gboolean be_verbose = FALSE;
  static gboolean use_certs = FALSE;
  static gchar *client_cert_path = NULL;
  static gchar *client_key_path = NULL;
  static gchar *client_ca_cert_path = NULL;
  static gchar *conf_file_path = NULL;
  static gchar *send_file_path = NULL;
  static gchar *manager_host_string = NULL;
  static gchar *manager_port_string = NULL;
  static gchar *omp_username = NULL;
  static gchar *omp_password = NULL;
  /* Shared command options. */
  static gchar *name = NULL;
  /* Command create-task. */
  static gboolean cmd_create_task = FALSE;
  static gchar *comment = NULL;
  static gchar *config = NULL;
  static gchar *target = NULL;
  /* Command delete-report. */
  static gboolean cmd_delete_report = FALSE;
  /* Command delete-task. */
  static gboolean cmd_delete_task = FALSE;
  /* Command get-report. */
  static gboolean cmd_get_report = FALSE;
  /* Command get-report-formats. */
  static gboolean cmd_get_report_formats = FALSE;
  /* Command get-omp-version. */
  static gboolean cmd_get_omp_version = FALSE;
  static gchar *format = NULL;
  /* Command get-tasks. */
  static gboolean cmd_get_tasks = FALSE;
  /* Command get-configs. */
  static gboolean cmd_get_configs = FALSE;
  /* Command get-targets. */
  static gboolean cmd_get_targets = FALSE;
  /* Command modify-task. */
  static gboolean cmd_modify_task = FALSE;
  static gboolean file = FALSE;
  /* Command start-task. */
  static gboolean cmd_start_task = FALSE;
  /* Filter string for get_reports */
  static gchar *filter = NULL;
  /* Command details */
  static gboolean cmd_details = FALSE;
  /* Command ping. */
  static gboolean cmd_ping = FALSE;
  static gint ping_timeout = DEFAULT_PING_TIMEOUT;
  /* Command given as XML. */
  static gchar *cmd_xml = NULL;
  /* The rest of the args. */
  static gchar **rest = NULL;
  /* Pretty print option. */
  static gboolean pretty_print = FALSE;

  GError *error = NULL;

  GOptionContext *option_context;
  static GOptionEntry option_entries[] = {
    /* Global options. */
    {"host", 'h', 0, G_OPTION_ARG_STRING, &manager_host_string,
     "Connect to manager on host <host>", "<host>"},
    {"port", 'p', 0, G_OPTION_ARG_STRING, &manager_port_string,
     "Use port number <number>", "<number>"},
    {"version", 'V', 0, G_OPTION_ARG_NONE, &print_version,
     "Print version.", NULL},
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &be_verbose,
     "Verbose messages (WARNING: may reveal passwords).", NULL},
    {"use-certs", 0, 0, G_OPTION_ARG_NONE, &use_certs,
     "Use client certificates to authenticate.", NULL},
    {"client-cert", 0, 0, G_OPTION_ARG_FILENAME, &client_cert_path,
     "Client certificate. Default: " CLIENTCERT, "<cert-file>"},
    {"client-key", 0, 0, G_OPTION_ARG_FILENAME, &client_key_path,
     "Client key. Default: " CLIENTKEY, "<key-file>"},
    {"client-ca-cert", 0, 0, G_OPTION_ARG_FILENAME, &client_ca_cert_path,
     "Client CA certificate. Default: " CACERT, "<cert-file>"},
    {"username", 'u', 0, G_OPTION_ARG_STRING, &omp_username,
     "OMP username", "<username>"},
    {"password", 'w', 0, G_OPTION_ARG_STRING, &omp_password,
     "OMP password", "<password>"},
    {"config-file", 0, 0, G_OPTION_ARG_FILENAME, &conf_file_path,
     "Configuration file for connection parameters.", "<config-file>"},
    {"prompt", 'P', 0, G_OPTION_ARG_NONE, &prompt,
     "Prompt to exit.", NULL},
    {"get-omp-version", 'O', 0, G_OPTION_ARG_NONE, &cmd_get_omp_version,
     "Print OMP version.", NULL},
    /* Shared command options. */
    {"name", 'n', 0, G_OPTION_ARG_STRING, &name,
     "Name for create-task.",
     "<name>"},
    /* Command create-task. */
    {"create-task", 'C', 0, G_OPTION_ARG_NONE, &cmd_create_task,
     "Create a task.", NULL},
    {"comment", 'm', 0, G_OPTION_ARG_STRING, &comment,
     "Comment for create-task.",
     "<name>"},
    {"config", 'c', 0, G_OPTION_ARG_STRING, &config,
     "Config for create-task.",
     "<config>"},
    {"target", 't', 0, G_OPTION_ARG_STRING, &target,
     "Target for create-task.",
     "<target>"},
    /* Command delete-report. */
    {"delete-report", 'E', 0, G_OPTION_ARG_NONE, &cmd_delete_report,
     "Delete one or more reports.", NULL},
    /* Command delete-task. */
    {"delete-task", 'D', 0, G_OPTION_ARG_NONE, &cmd_delete_task,
     "Delete one or more tasks.", NULL},
    /* Command get-report. */
    {"get-report", 'R', 0, G_OPTION_ARG_NONE, &cmd_get_report,
     "Get report of one task.", NULL},
    {"get-report-formats", 'F', 0, G_OPTION_ARG_NONE, &cmd_get_report_formats,
     "Get report formats. (OMP 2.0 only)", NULL},
    {"format", 'f', 0, G_OPTION_ARG_STRING, &format,
     "Format for get-report.",
     "<format>"},
    {"filter", 0, 0, G_OPTION_ARG_STRING, &filter,
     "Filter string for get-report",
     "<string>"},
    /* Command get-tasks. */
    {"get-tasks", 'G', 0, G_OPTION_ARG_NONE, &cmd_get_tasks,
     "Get status of one, many or all tasks.", NULL},
    /* Command get-configs. */
    {"get-configs", 'g', 0, G_OPTION_ARG_NONE, &cmd_get_configs,
     "Get configs.", NULL},
    /* Command get-targets. */
    {"get-targets", 'T', 0, G_OPTION_ARG_NONE, &cmd_get_targets,
     "Get targets.", NULL},
    /* Pretty printing for "direct" xml (in combination with -X). */
    {"pretty-print", 'i', 0, G_OPTION_ARG_NONE, &pretty_print,
     "In combination with -X, pretty print the response.", NULL},
    /* Command start-task. */
    {"start-task", 'S', 0, G_OPTION_ARG_NONE, &cmd_start_task,
     "Start one or more tasks.", NULL},
    /* Command modify-task. */
    {"modify-task", 'M', 0, G_OPTION_ARG_NONE, &cmd_modify_task,
     "Modify a task.", NULL},
    /* Command ping. */
    {"ping", 0, 0, G_OPTION_ARG_NONE, &cmd_ping,
     "Ping OMP server", NULL},
    {"timeout", 't', 0, G_OPTION_ARG_INT, &ping_timeout,
     "Wait <number> seconds for OMP ping response", "<number>"},
    {"file", 0, 0, G_OPTION_ARG_NONE, &file,
     "Add text in stdin as file on task.", NULL},
    /* Command as XML. */
    {"xml", 'X', 0, G_OPTION_ARG_STRING, &cmd_xml,
     "XML command (e.g. \"<help/>\").  \"-\" to read from stdin.",
     "<command>"},
    {"send-file", 0, 0, G_OPTION_ARG_FILENAME, &send_file_path,
     "Replace SENDFILE in xml with base64 of file.", "<file>"},
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &rest,
     NULL, NULL},
    /* Enable details */
    {"details", 0, 0, G_OPTION_ARG_NONE, &cmd_details,
     "Enable detailed view.", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

  if (setlocale (LC_ALL, "") == NULL)
    {
      printf ("Failed to setlocale\n\n");
      exit (EXIT_FAILURE);
    }

  option_context =
    g_option_context_new ("- OpenVAS OMP Command Line Interface");
  g_option_context_add_main_entries (option_context, option_entries, NULL);
  if (!g_option_context_parse (option_context, &argc, &argv, &error))
    {
      printf ("%s\n\n", error->message);
      exit (EXIT_FAILURE);
    }

  /* Check for Windows style help request */
  if (rest != NULL && *rest != NULL)
    {
      if (g_strstr_len (*rest, -1, "/?") != NULL)
        {
          printf ("%s", g_option_context_get_help (option_context, TRUE, NULL));
          exit (EXIT_SUCCESS);
        }
    }

  if (print_version)
    {
      printf ("OMP Command Line Interface %s\n", OPENVASCLI_VERSION);
      printf ("Copyright (C) 2010-2016 Greenbone Networks GmbH\n");
      printf ("License GPLv2+: GNU GPL version 2 or later\n");
      printf
        ("This is free software: you are free to change and redistribute it.\n"
         "There is NO WARRANTY, to the extent permitted by law.\n\n");
      exit (EXIT_SUCCESS);
    }

  /* Check that one and at most one command option is present. */
  {
    int commands;
    commands =
      (int) cmd_create_task + (int) cmd_delete_report + (int) cmd_delete_task +
      (int) cmd_get_report + (int) cmd_get_report_formats +
      (int) cmd_get_tasks + (int) cmd_modify_task + (int) cmd_start_task +
      (int) cmd_get_targets + (int) cmd_get_omp_version + (int) cmd_get_configs +
      (int) cmd_ping + (int) (cmd_xml != NULL);
    if (commands == 0)
      {
        fprintf (stderr, "One command option must be present.\n");
        exit (EXIT_FAILURE);
      }
    if (commands > 1)
      {
        fprintf (stderr, "Only one command option must be present.\n");
        exit (EXIT_FAILURE);
      }
  }

  /* Setup the connection structure from the arguments and conf file.
   * Precedence of values is the following:
   * 1) command line argument (e.g. --port) 2) conf file 3) default */

  if (conf_file_path == NULL)
    conf_file_path = g_build_filename (g_get_home_dir (), "omp.config", NULL);
  connection = connection_from_file (conf_file_path);
  g_free (conf_file_path);

  if (manager_host_string != NULL)
    connection->host_string = manager_host_string;
  else if (connection->host_string == NULL)
    connection->host_string = OPENVASMD_ADDRESS;

  if (manager_port_string != NULL)
    connection->port = atoi (manager_port_string);
  else if (connection->port_string != NULL)
    connection->port = atoi (connection->port_string);
  else
    connection->port = OPENVASMD_PORT;

  if (connection->port <= 0 || connection->port >= 65536)
    {
      fprintf (stderr, "Manager port must be a number between 0 and 65536.\n");
      exit (EXIT_FAILURE);
    }

  connection->use_certs = use_certs;
  if (omp_username != NULL)
    connection->username = omp_username;
  else if (connection->username == NULL)
    connection->username = g_strdup (g_get_user_name ());

  if (client_cert_path != NULL)
    connection->client_cert_path = client_cert_path;
  else if (connection->client_cert_path == NULL)
    connection->client_cert_path = CLIENTCERT;

  if (client_key_path != NULL)
    connection->client_key_path = client_key_path;
  else if (connection->client_key_path == NULL)
    connection->client_key_path = CLIENTKEY;

  if (client_ca_cert_path != NULL)
    connection->client_ca_cert_path = client_ca_cert_path;
  else if (connection->client_ca_cert_path == NULL)
    connection->client_ca_cert_path = CACERT;

  if (ping_timeout < 0)
    ping_timeout = DEFAULT_PING_TIMEOUT;
  connection->timeout = ping_timeout;

  if (omp_password != NULL)
    connection->password = omp_password;
  else if (connection->password == NULL && !connection->use_certs && !cmd_ping
           && !cmd_get_omp_version)
    {
      gchar *pw = NULL;
      size_t n;

      printf ("Enter password: ");
      int ret = read_password (&pw, &n, stdin);
      printf ("\n");

      if (ret < 0)
        {
          fprintf (stderr, "Failed to read password from console!\n");
          exit (EXIT_FAILURE);
        }

      /* Remove the trailing newline character. */
      pw[ret - 1] = '\0';

      if (strlen (pw) > 0)
        connection->password = pw;
      else
        {
          fprintf (stderr, "Password must be set.\n");
          exit (EXIT_FAILURE);
        }
    }

  if (be_verbose)
    {
      const char *s;

      /** @todo Other modules ship with log level set to warning. */
      printf ("\nWARNING: Verbose mode may reveal passwords!\n\n");
      printf ("Will try to connect to host %s, port %d...\n",
              connection->host_string, connection->port);

      /* Enable GNUTLS debugging if the envvar, as used by the
         standard log functions, is set.  */
      if ((s=getenv ("OPENVAS_GNUTLS_DEBUG")))
        {
          gnutls_global_set_log_function (my_gnutls_log_func);
          gnutls_global_set_log_level (atoi (s));
        }
    }
  else
    {
#ifndef _WIN32
      g_log_set_default_handler (openvas_log_silent, NULL);
#endif
    }

  /* Run the single command. */

  if (cmd_create_task)
    {
      char *id = NULL;

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      if (omp_create_task
          (&(connection->session), name ? name : "unnamed task",
           config ? config : "Full and fast", target ? target : "Localhost",
           comment ? comment : "", &id))
        {
          fprintf (stderr, "Failed to create task.\n");
          manager_close (connection);
          exit (EXIT_FAILURE);
        }

      printf ("%s", id);
      putchar ('\n');

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_delete_report)
    {
      gchar **point = rest;

      if (point == NULL || *point == NULL)
        {
          fprintf (stderr, "delete-report requires at least one argument.\n");
          exit (EXIT_FAILURE);
        }

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      while (*point)
        {
          if (omp_delete_report (&(connection->session), *point))
            {
              fprintf (stderr, "Failed to delete report %s, exiting.\n",
                       *point);
              manager_close (connection);
              exit (EXIT_FAILURE);
            }
          point++;
        }

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_delete_task)
    {
      gchar **point = rest;

      if (point == NULL || *point == NULL)
        {
          fprintf (stderr, "delete-task requires at least one argument.\n");
          exit (EXIT_FAILURE);
        }

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      while (*point)
        {
          if (omp_delete_task (&(connection->session), *point))
            {
              fprintf (stderr, "Failed to delete task.\n");
              manager_close (connection);
              exit (EXIT_FAILURE);
            }
          point++;
        }

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_get_tasks)
    {
      gchar **point = rest;
      entity_t status;

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      if (point)
        while (*point)
          {
            omp_get_task_opts_t opts;

            opts = omp_get_task_opts_defaults;
            opts.task_id = *point;
            opts.details = 1;
            opts.actions = "g";

            if (omp_get_task_ext (&(connection->session), opts, &status))
              {
                fprintf (stderr, "Failed to get status of task %s.\n", *point);
                manager_close (connection);
                exit (EXIT_FAILURE);
              }
            else
              {
                if (print_tasks (status->entities))
                  {
                    manager_close (connection);
                    exit (EXIT_FAILURE);
                  }
              }

            point++;
          }
      else
        {
          omp_get_tasks_opts_t opts;

          opts = omp_get_tasks_opts_defaults;
     	  if(cmd_details)
          	opts.details = 1;
	  else 
		opts.details = 0;
          opts.filter = "permission=any owner=any rows=-1";

          if (omp_get_tasks_ext (&(connection->session), opts, &status))
            {
              fprintf (stderr, "Failed to get status of all tasks.\n");
              manager_close (connection);
              exit (EXIT_FAILURE);
            }
          if (print_tasks (status->entities))
            {
              manager_close (connection);
              exit (EXIT_FAILURE);
            }
        }

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_get_configs)
    {
      entity_t status;

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      if (get_configs (&(connection->session), &status))
        {
          fprintf (stderr, "Failed to get configs.\n");
          exit (EXIT_FAILURE);
        }
      if (print_configs (status->entities))
        {
          manager_close (connection);
          exit (EXIT_FAILURE);
        }

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_get_targets)
    {
      entity_t status;

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      if (omp_get_targets (&(connection->session), NULL, 0, 0, &status))
        {
          fprintf (stderr, "Failed to get targets.\n");
          exit (EXIT_FAILURE);
        }
      if (print_targets (status->entities))
        {
          manager_close (connection);
          exit (EXIT_FAILURE);
        }

      manager_close (connection);
      exit_status = 0;
    }
  else if (cmd_get_report)
    {
      gchar **report_ids = rest;

      if (report_ids == NULL || *report_ids == NULL)
        {
          fprintf (stderr, "get-report requires one argument.\n");
          exit (EXIT_FAILURE);
        }

      if (manager_open (connection))
        exit (EXIT_FAILURE);
      exit_status = manager_get_reports (connection, report_ids, format, filter);
      if (exit_status == 0)
        manager_close (connection);
    }
  else if (cmd_get_report_formats)
    {
      if (manager_open (connection))
        exit (EXIT_FAILURE);
      exit_status = manager_get_report_formats (connection);
      if (exit_status == 0)
        manager_close (connection);
    }
  else if (cmd_get_omp_version)
    {
      gchar *version = NULL;
      if (manager_open (connection))
        exit (EXIT_FAILURE);
      exit_status = manager_get_omp_version (connection, &version);
      printf ("Version: %s\n", version);
      g_free (version);
      if (exit_status == 0)
        manager_close (connection);
    }
  else if (cmd_ping)
    {
      if (manager_open (connection))
        {
          fprintf (stderr, "OMP ping failed: Failed to establish connection.\n");
          exit_status = 1;
        }
      else
        {
          int res;
          /* Returns 0 on success, 1 if manager closed connection, 2 on
             timeout, -1 on error */
          res = omp_ping (&(connection->session), connection->timeout);
          if (res == 0)
            {
              fprintf (stdout, "OMP ping was successful.\n");
              exit_status = 0;
            }
          else if (res == 1)
            {
              fprintf (stderr, "OMP ping failed: Server closed connection.\n");
              exit_status = 1;
            }
          else if (res == 2)
            {
              fprintf (stderr, "OMP ping failed: Timeout.\n");
              exit_status = 1;
            }
          else
            {
              fprintf (stderr, "OMP ping failed: Unknown error.\n");
              exit_status = 1;
            }
        }
      if (exit_status == 0)
        manager_close (connection);
    }
  else if (cmd_modify_task)
    {
      gchar **point = rest;
      gchar *content;
      gsize content_len;

      if (point == NULL || *point == NULL)
        {
          fprintf (stderr, "modify-task requires one argument.\n");
          exit (EXIT_FAILURE);
        }

      if (name == NULL)
        {
          fprintf (stderr,
                   "modify-task requires the name option (path to file).\n");
          exit (EXIT_FAILURE);
        }

      if (file == FALSE)
        {
          fprintf (stderr, "modify-task requires the file option.\n");
          exit (EXIT_FAILURE);
        }

      if (file)
        {
          GIOChannel *stdin_channel;

          if (manager_open (connection))
            exit (EXIT_FAILURE);
          /* Mixing stream and file descriptor IO might lead to trouble. */
          error = NULL;
          stdin_channel = g_io_channel_unix_new (fileno (stdin));
          g_io_channel_read_to_end (stdin_channel, &content, &content_len,
                                    &error);
          g_io_channel_shutdown (stdin_channel, TRUE, NULL);
          g_io_channel_unref (stdin_channel);
          if (error)
            {
              fprintf (stderr, "failed to read from stdin: %s\n",
                       error->message);
              g_error_free (error);
              exit (EXIT_FAILURE);
            }

#if 0
          /** todo As in get-report, this is how the commands will work. */
          exit_status =
            manager_modify_task_file (connection, *point, name, content,
                                      content_len, error);
#else
          if (omp_modify_task_file
              (&(connection->session), *point, name, content, content_len))
            {
              g_free (content);
              fprintf (stderr, "Failed to modify task.\n");
              manager_close (connection);
              exit (EXIT_FAILURE);
            }

          manager_close (connection);
          exit_status = 0;
#endif
        }
    }
  else if (cmd_start_task)
    {
      gchar **point = rest;

      if (point == NULL || *point == NULL)
        {
          fprintf (stderr, "start-task requires at least one argument.\n");
          exit (EXIT_FAILURE);
        }

      if (manager_open (connection))
        exit (EXIT_FAILURE);

      while (*point)
        {
          char *report_id;
          if (omp_start_task_report
              (&(connection->session), *point, &report_id))
            {
              fprintf (stderr, "Failed to start task.\n");
              manager_close (connection);
              exit (EXIT_FAILURE);
            }
          printf ("%s\n", report_id);
          free (report_id);
          point++;
        }
      exit_status = 0;

      manager_close (connection);
    }
  else if (cmd_xml)
    {
      if (manager_open (connection))
        exit (EXIT_FAILURE);

      if (send_file_path)
        {
          char *new_xml = cmd_xml;
          if (replace_send_file_xml (&new_xml, send_file_path))
            exit (EXIT_FAILURE);
          g_free (cmd_xml);
          cmd_xml = new_xml;
        }

      /** @todo Move to connection_t and manager_open. */
      if (prompt)
        {
          fprintf (stderr, "Connected, press a key to continue.\n");
          getchar ();
        }

      if (strcmp (cmd_xml, "-") == 0)
        {
          GError *error;
          gchar *content;
          gsize content_len;
          GIOChannel *stdin_channel;

          /* Mixing stream and file descriptor IO might lead to trouble. */
          error = NULL;
          stdin_channel = g_io_channel_unix_new (fileno (stdin));
          g_io_channel_read_to_end (stdin_channel, &content, &content_len,
                                    &error);
          g_io_channel_shutdown (stdin_channel, TRUE, NULL);
          g_io_channel_unref (stdin_channel);
          if (error)
            {
              fprintf (stderr, "Failed to read from stdin: %s\n",
                       error->message);
              g_error_free (error);
              exit (EXIT_FAILURE);
            }

          g_free (cmd_xml);
          cmd_xml = content;
        }

      if (be_verbose)
        printf ("Sending to manager: %s\n", cmd_xml);

      if (openvas_server_sendf (&(connection->session), "%s", cmd_xml) == -1)
        {
          manager_close (connection);
          fprintf (stderr, "Failed to send_to_manager.\n");
          exit (EXIT_FAILURE);
        }

      /* Read the response. */

      entity_t entity = NULL;
      if (read_entity (&(connection->session), &entity))
        {
          fprintf (stderr, "Failed to read response.\n");
          manager_close (connection);
          exit (EXIT_FAILURE);
        }

      if (be_verbose)
        printf ("Got response:\n");
      if (pretty_print == FALSE)
        print_entity (stdout, entity);
      else
        print_entity_format (entity, GINT_TO_POINTER (2));
      printf ("\n");

      /* Cleanup. */

      /** @todo Move to connection_t and manager_open. */
      if (prompt)
        {
          fprintf (stderr, "Press a key when done.\n");
          getchar ();
        }

      manager_close (connection);
      free_entity (entity);

      exit_status = 0;
    }
  else
    /* The option processing ensures that at least one command is present. */
    assert (0);

  /* Exit. */

  if (be_verbose)
    {
      if (exit_status)
        printf ("Command failed.\n");
      else
        printf ("Command completed successfully.\n");
    }

  exit (exit_status);
}
