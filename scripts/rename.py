import os
import sys

argCount = len(sys.argv)

# Get the passed in directory or the directory of the script.
apath = sys.argv[1] if argCount == 2 else sys.argv[0]

directory = os.path.dirname(os.path.realpath(apath))
for subdir, dirs, files in os.walk(directory):
    for filename in files:
        if filename.find('.Lib') > 0:
            # Get the path of the sub dir.
            subdirectoryPath = os.path.relpath(subdir, directory)
            # Get the path to your file.
            filePath = os.path.join(subdirectoryPath, filename)
            # Create the new name.
            newFilePath = filePath.replace(filename, filename.lower())
            # Rename the file.
            os.rename(filePath, newFilePath)
