srcFolder:=src
buildFolder:=build

CFLAGS:=-Wall -pedantic -Werror -std=c11 -g -I${srcFolder} -I${srcFolder}/impl/com-port_win

${buildFolder}/lab3: ${srcFolder}/messager.c ${srcFolder}/impl/com-port_win/messager_impl.c 
	mkdir -p ${buildFolder}
	gcc ${CFLAGS} $^ -o $@