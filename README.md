# Sistemas-Operativos-SO-
Todo lo relacionado con lo visto o requerido para el ramo de sistemas operativos (En la UDP)

Para referencia mía de setear el repositorio de manera conveniente (no hace falta leer esto):
ssh-keygen -t ed25519 -C "correo@algo.com"        (enter para ruta predeterminada)
enter para proseguir sin contraseña
PARA ssh-agent (mejor práctica)
ssh-keygen -p -f ~/.ssh/id_ed25519 (para cambiar sin contraseña y poner una)

eval "$(ssh-agent -s)"     (para iniciar la ssh-agent en la sesión actual, admin de llaves en 2do plano)
ssh-add ~/.ssh/id_ed25519    (agregar contraseña a la llave solo esta vez (por esta sesión))
ESTO SE LIMITA A LA TERMINAL EN LA QUE SE REALIZÓ ESTO, SE PUEDE AUTOMATIZAR? SI, PERO ME DA LA LATA HACER ESO.

git init    en terminal en una carpeta para vincularlo al respositorio

git branch -M main
git remote add origin git@github.com:VicenteMuhr/Sistemas-Operativos-SO-.git

git add planificador.h planificador.c
git commit -m "algo"
git push -u origin main

git config --global user.email "you@example.com"
git config --global user.name "Your Name" 

eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

ssh -T git@github.com

cat ~/.ssh/id_ed25519.pub
RESULTADO DEL CAT copiar y pegar en add new en los settings ssh del perfil de github.

git pull origin main --rebase      y de ahí hacer el push, porq a veces no deja si tiene un readme o similar en el repositorio de base.

git push -u origin main


sudo systemctl restart NetworkManager      por si la vm de kali anda chingando, algo funcionó
