#ifndef RAPTOR_NOTIFY_H
#define RAPTOR_NOTIFY_H

#include <stdbool.h>
#include <stddef.h>

/* Prototypes Section 15 (Fonctions 86 et 87) */
void raptor_notify_send_desktop(const char* title, const char* body, bool is_critical);
void raptor_notify_evaluate_threats(int current_threats, const char* primary_threat_name);

#endif /* RAPTOR_NOTIFY_H */
