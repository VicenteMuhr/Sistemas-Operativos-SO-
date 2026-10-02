Vicente Muhr Souper

eval "$(ssh-agent -s)"        (esto nomas es para yo acordarme como iniciar bien la sesión en terminal)
ssh-add ~/.ssh/id_ed25519
ssh -T git@github.com

Planificador Dieciochero - Tarea 1 Sistemas Operativos

Este proyecto implementa un "Planificador Dieciochero", un simulador y orquestador de procesos concurrente escrito en C17. El sistema modela actividades como un Grafo Acíclico Dirigido (DAG) y las ejecuta utilizando procesos pesados (fork), comunicándose mediante tuberías (pipes), y manejando señales para la tolerancia a fallos y abortos del sistema.

para ver lo de errores
gcc -Wall -Wextra -std=c17 planificador.c main.c -o programa -lpthread

El programa recibe dos argumentos: el archivo de texto con el plan de actividades y el límite de concurrencia K
./programa plan.txt <K> (yo usé 2 para las pruebas nomas, pero funciona con eso al menos)

Funciones usadas:
- Parseo y Modelado del DAG: Lee plan.txt en dos pasadas. La primera extrae los nodos y tiempos (asignando aleatorios si no existen), y la segunda arma las aristas, calculando el in_degree (dependencias previas) y guardando los índices de los hijos sucesores.
- Control de Concurrencia (K): Mediante un bucle monitoreado por waitpid, el padre lanza tareas con fork() asegurando que nunca haya más de $K$ procesos activos simultáneamente.
- Paso de Mensajes (Pipes): Cada proceso hijo tiene un pipe anónimo. Al terminar su ejecución simulada (usleep), el hijo escribe un mensaje de éxito en el pipe. El padre lee este mensaje para confirmar la finalización y procede a reducir el in_degree de los nodos dependientes
- Aislamiento de Errores: Se implementó una probabilidad de fallo interno en los procesos hijos (15%). Si un hijo termina con EXIT_FAILURE, el padre lo detecta y ejecuta una función recursiva que marca toda la rama descendiente del DAG como ESTADO_CANCELADO, permitiendo que el resto del sistema siga operando.
- Manejo de Señales (SIGINT): Se registró un manejador para la señal SIGINT (Ctrl+C). Al activarse, una bandera atómica rompe el bucle principal del planificador y el padre envía una señal SIGKILL a todos los hijos actualmente en ejecución, simulando la inspección de la Seremi.

Justificación diseño:
- Arreglo Contiguo para el DAG: En lugar de usar punteros complejos y memoria fragmentada, el grafo se guardó en un arreglo dinámico (plan->actividades). Las dependencias se manejan guardando el índice entero de los hijos. Esto facilita la búsqueda de tareas.
- Manejo del bucle principal: Se utilizó un contador de actividades_finalizadas que se compara contra el total de tareas. Para evitar deadlocks (bloqueos) cuando ocurre un error, la función recursiva de cancelación recibe un puntero a este contador y suma las tareas canceladas, asumiendo que ya fueron "resueltas" por el planificador.
- Monitoreo sin Busy-Waiting: Se empleó waitpid(-1, &status, 0) para que el padre duerma y solo despierte cuando un hijo cambie de estado, maximizando la eficiencia de la CPU sin caer en esperas activas.
- Separación de responsabilidades: La lógica del grafo y procesos fue encapsulada en planificador.c y su respectiva cabecera .h, manteniendo a main.c limpio y responsable únicamente de la validación de argumentos.

