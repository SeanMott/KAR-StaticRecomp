#gets the FMT logging library

import GitOperations

#generate the Premake string for adding the include dir
def GeneratePremake_IncludePath():
    return """"Venders/FMT/include","""

#clones FMT
def GetDep_FMT():
    GitOperations.GitClone("https://github.com/fmtlib/fmt.git", "FMT")