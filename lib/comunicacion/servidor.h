#pragma once

// ==========================================================
//  SERVIDOR WEB - modo AP
//  Sirve la interfaz HTML y expone endpoints para la app:
//    GET /            -> pagina
//    GET /medir       -> JSON con el valor actual
//    GET /modo?m=N    -> cambia el modo de operacion
// ==========================================================

void servidor_setup();
void servidor_loop(); // llamar en cada iteracion del loop principal