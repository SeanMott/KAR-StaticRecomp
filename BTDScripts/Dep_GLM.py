#gets the Open GL Math library

import GitOperations

#generate the Premake string for adding the include dir
def GeneratePremake_IncludePath():
    return """"Venders/GLM","""

#clones GLM
def GetDep_GLM():
    GitOperations.GitClone("https://github.com/g-truc/glm.git", "GLM")