#gets the STB header only library

import GitOperations

#generate the Premake string for adding the include dir
def GeneratePremake_IncludePath():
    return """"Venders/STB","""

#clones STB
def GetDep_STB():
    GitOperations.GitClone("https://github.com/nothings/stb.git", "STB")