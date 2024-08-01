#gets ImGUI

import GitOperations

#generate the Premake string for adding the include dir
def GeneratePremake_IncludePath():
    return "\"Venders/ImGUI\""

#clones ImGUI
def GetDep_ImGUI():
    GitOperations.GitClone("https://github.com/ocornut/imgui.git", "ImGUI")