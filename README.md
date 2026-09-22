# Sistemas-Operativos-SO-
Todo lo relacionado con lo visto o requerido para el ramo de sistemas operativos (En la UDP)

Para referncia mía de setear el repositorio de manera conveniente (no hace falta leer esto):
ssh-keygen -t ed25519 -C "correo@algo.com"        (enter para ruta predeterminada)
enter para proseguir sin contraseña
PARA ssh-agent (mejor práctica)
ssh-keygen -p -f ~/.ssh/id_ed25519 (para cambiar sin contraseña y poner una)

eval "$(ssh-agent -s)"     (para iniciar la ssh-agent en la sesión actual, admin de llaves en 2do plano)
ssh-add ~/.ssh/id_ed25519    (agregar contraseña a la llave solo esta vez (por esta sesión))
ESTO SE LIMITA A LA TERMINAL EN LA QUE SE REALIZÓ ESTO, SE PUEDE AUTOMATIZAR? SI, PERO ME DA LA LATA HACER ESO.
