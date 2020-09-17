"""
Build in parallel the different build
"""
import subprocess, os, sys, asyncio, shutil

# global returncode
returncode = 0 # = except if changed by a failed bat
# list of build categories
builds = [ "test", "keil", "lx", "nios", "soc", "x64"]

# path of the build batch
path = os.path.dirname(os.path.realpath(__file__))
builder =os.path.join(path, 'msbuild_vsprojects.bat')


async def run(build):
    "async run msbuild_vsprojects build and print output correctly"
    global returncode
    buffer = b""
    # run the builds by category throw the batch file
    proc = await asyncio.create_subprocess_shell(
        " ".join([builder, build, "-no-delete"]),
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.STDOUT)

    try:
        # read all line for the current process
        line="\n"
        build = bytes(build, 'ascii')
        while line:
            # async read to ensure output of all builds
            line = (await proc.stdout.readline()) #.decode()
            buffer += b"% 4s: %s" % (build, line)
            # write output only when a build is ended to avoid mismatchs
            if line.startswith(b"======="):
                sys.stdout.buffer.write(buffer)
                sys.stdout.buffer.flush()
                buffer = b""

        # ensure the process terminate
        await proc.wait()
        # if failure, store it in returncode
        if proc.returncode:
            returncode = proc.returncode
        
    except:
        proc.terminate()
        raise


# maximum number of build in //
MAXBUILD = 3
async def main():
    # reinstall packages
    subprocess.run(["cmd", "/c", builder, "dotnet"])

    # remove log
    log = os.path.join(path, "log")
    if os.path.exists(log):
        shutil.rmtree(log)

    pending = set()
    # start all batch builds in parallel by MAXBUILD
    for last in range(MAXBUILD):
        pending.add(asyncio.create_task(run(builds[last])))
    # and wait for the end of all builds
    while pending:
        done, pending = await asyncio.wait(pending, return_when=asyncio.FIRST_COMPLETED)
        while len(pending) < MAXBUILD and last+1 < len(builds):
            last += 1
            pending.add(asyncio.create_task(run(builds[last])))


# first argument will overwrite the number of // build
if len(sys.argv) > 1:
    MAXBUILD = int(sys.argv[1])
asyncio.run(main())
# return the return code of builds
sys.exit(returncode)