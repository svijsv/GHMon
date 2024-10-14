Import('env')
from os.path import join, realpath

for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "uHAL_PLATFORM":
        #env.Append(CPPPATH=[realpath(join("platform", item[1]))])
        env.Replace(SRC_FILTER=["+<*>", "-<platform/>", "+<platform/%s>" % item[1]])
        break
