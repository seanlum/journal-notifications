#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libnotify/notify.h>
#include <systemd/sd-journal.h>

void make_notification(const char *notification_text) {
    notify_init(notification_text);
    NotifyNotification *new_notification = notify_notification_new("journalctl alert: ", notification_text, "");
    notify_notification_set_timeout(new_notification, NOTIFY_EXPIRES_NEVER);
    notify_notification_set_urgency(new_notification, NOTIFY_URGENCY_NORMAL);
    notify_notification_show(new_notification, NULL);
    g_object_unref(G_OBJECT(new_notification));
    notify_uninit();
}

int enumerate_fields(sd_journal *journal, int *field_match, const char *identifier_name) {
    const char *field;
    const char *field_data;
         size_t field_data_len;
            int field_err;
    SD_JOURNAL_FOREACH_FIELD(journal, field) {
        field_err = sd_journal_get_data(journal, field, (const void **) &field_data, &field_data_len);
         if (-field_err == ENOENT) {
            continue;
        } else if (field_err < 0) {
            fprintf(stdout, "Failed to retrieve field (%s): %s\n", field, strerror(-field_err));
        } else {
            char field_name[4096];
            snprintf(field_name, 4096, "%s=%s\0", field, identifier_name);
            if ((strcmp(field_name, field_data)) == 0) {
                return 0;
            }
        }
    }
    return -1;
}


int get_next(sd_journal *journal, long long *message_count, const char **identifier_names, int identifiers_length) {
    *message_count = *message_count + 1;
    if (sd_journal_next(journal) == 0) {
        return 0;
    }
    int matches_identifier = -1;
    for (int i=0; i<identifiers_length; i++) {
        if (matches_identifier == -1) {
            matches_identifier = enumerate_fields(journal, &matches_identifier, identifier_names[i]);
        }
        if (matches_identifier != -1) {
            break;
        }
    }
    if (matches_identifier == 0) {
        return 3;
    }
    if (*message_count > 1000) {
        *message_count = 0;
        return 2;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    int sd_journal_open_err; 
    sd_journal *journal;
    sd_journal *new_journal;
    sd_journal_open_err = sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY);
    if (sd_journal_open_err < 0) {
        fprintf(stderr, "Failed to open journal: %s\n", strerror(sd_journal_open_err));
        return 1;
    }

    int identifiers_length = argc-1;
    const char *identifiers[identifiers_length];
    for (int i=0; i<identifiers_length; i++) {
        identifiers[i] = argv[i+1];
    }

    if (sd_journal_seek_tail(journal) < 0) {
        fprintf(stderr, "Could not seek to the most recent messages.\n");
        return 1;
    } 

    long long message_count = 0;
    int status;
    int reopen_status;
    while(1) {
        status = get_next(journal, &message_count, identifiers, identifiers_length);
        if (status == 0) {
            continue;
        } else if (status == 1) {
            continue;
        } else if (status == 2) {

                  message_count = 0;
            char *cursor;
              int cursor_error = sd_journal_get_cursor(journal, &cursor);

            if (cursor_error < 0) {
                fprintf(stderr, "Could not get cursor for open journal: %s\n", strerror(-cursor_error));
            }

            sd_journal_close(journal);
            int open_error = sd_journal_open(&new_journal, SD_JOURNAL_LOCAL_ONLY);
            journal = new_journal;
            sd_journal_next(new_journal);
            
            if (open_error < 0) {
                fprintf(stderr, "Could not reopen journal: %s\n", strerror(-cursor_error));
            } else {
                int cursor_set_error = sd_journal_seek_cursor(new_journal, cursor);
                if (cursor_error < 0) {
                    fprintf(stderr, "Could not set cursor: %s\nCursor: %s\n", strerror(-cursor_error), cursor);
                }
            }

        } else if (status == 3) {
            const char *message;
                 size_t message_len;
            const char *field = "MESSAGE";
                 size_t field_len = strnlen(field, 9);
                    int message_err = sd_journal_get_data(journal, field, (const void **) &message, &message_len);

            if (message_err < 0) {
                fprintf(stderr, "Failed to read message id: %s\n", strerror(message_err));
            } else {
                int final = message_len - field_len - 1;
                char output_message[final];
                for (int i=0; i<final; i++) {
                    output_message[i] = message[i+field_len+1];
                    if ((i+1) >= final) {
                        output_message[i+1] = '\0';
                    }
                }
                pid_t new_pid = fork();
                if (new_pid==-1) {
                    fprintf(stderr, "Could not fork\n");
                } 
                if (new_pid == 0) {
                    make_notification(output_message);
                    exit(0);
                } else {
                    continue;
                }
            }
        }
    }
    sd_journal_close(journal);
    return 0;
}