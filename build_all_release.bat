ECHO Generating project files:
cmake . 
ECHO Building:
cmake --build . --config Release
ECHO Compiling shaders:
cd Demo
python compile_shaders.py
PAUSE