import subprocess
Import("env")

git_commit = subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip() 
git_commit = git_commit[0:8]
git_commit = str(git_commit)
git_flag = str('-D__GIT_HASH__=\\"') + git_commit[2:10] + '\\"'

print("Uploading with following git commit ID: ", git_flag)

env.Append(
  BUILD_FLAGS=[
      git_flag
  ]
)

#env.AddPreAction("$BUILD_DIR/src/main.cpp.o", buildprog)