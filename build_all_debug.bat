ECHO Generating project files:
cmake . 
ECHO Building:
cmake --build . --config Debug
ECHO Compiling shaders:
cd Demo
python compile_shaders.py
PAUSE