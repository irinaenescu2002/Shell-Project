# Shell-Project
Proiect pentru SIsteme de Operare ce urmareste implementarea unui Shell (proiect de echipa). 

# Team members and contributions:
### Irina Enescu:
- added the manual with all the shell commands
- command parsing 
- changing the current directory
- a shell menu that decides which command will be executed
- shell interface 
- code formatting
- added the functionality of using logical operators (&&) and (||)

### Bogdan Putinelu:
- suspending a program
- executing a job in the background (&)
- chaining commands with semi-colon (;)
- added the functionality of using logical operators together (ex. cmd1 && cmd2 || cmd3)
- added the functionality of semi-colon inside logical expressions (&&, ||)

### Robert Udrea:
- display a new line in shell
- reading input from the keyboard
- clear command 
- arrow navigation in history or display history
- added the functionality of using pipe
- unlimited power function which executes classic commands

# How to use this masterpiece
> The program runs correctly on Linux or MAC.
1. Save the shell.c file in a folder.

2. Open the folder in the terminal.

3. Run the following commands:
```sh
gcc shell.c -o shell -lreadline
./shell
```
- If you don't have the GCC or the Readline library compiler installed, run the following commands in the terminal:
```sh
sudo apt update
sudo apt install build-essential
sudo apt install gcc
sudo apt-get install libreadline-dev
```
- To validate that the GCC compiler is successfully installed, run the following command in the terminal:
```sh
gcc --version
```

4. Our Shell works! Type man to see all the commands:
```sh
man
```

5. Here are some examples:
```sh
touch firstFile.txt
ls
touch secondFile.txt
ls
pwd
rm secondFile.txt
ls
mkdir firstDir
ls
rmdir firstDir
ls
echo hello
clear
history
quit
```
> Have fun!
