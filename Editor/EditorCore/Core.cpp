#include "Core.h"


static SmoothieEditor::Core editor_core;
SmoothieEditor::Core& SmoothieEditor::getCore()
{
    return editor_core;
}
