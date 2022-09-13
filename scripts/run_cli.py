import subprocess
import json
import sys
import unittest
import os

class TestConfigJsonMethods(unittest.TestCase):
    def test_config(self):
        covergeName = os.environ['LLVM_PROFILE_FILE']
        if(len(sys.argv)== 2):
            with open("cli-test-cases/config.json", 'r') as f:
                testConfigData = json.load(f)
                for idx, test in enumerate(testConfigData):
                    cliCommand = [test["execpath"]]
                    if len(covergeName) > 0:
                        os.environ["LLVM_PROFILE_FILE"] = "{}{}.profraw".format(covergeName, idx)
                    cliCommand.extend(test["arguments"])
                    process = subprocess.Popen(cliCommand, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    stdout, stderr = process.communicate()
                    if(test.get("expected")):
                        if type(test["expected"]) == int:
                            self.assertEqual(int(stdout), test["expected"])
                        if type(test["expected"]) == bool:
                            self.assertEqual(bool(stdout), test["expected"])
                        if type(test["expected"]) == str:
                            self.assertEqual(stdout.decode("utf-8"), test["expected"])
                    if(test.get("error")):
                        self.assertEqual(stderr.decode("utf-8"), test["error"])
                    if(test.get("comparePath")):
                        with open(test["comparePath"]) as f:
                            contents = f.read()
                            self.assertEqual(stdout.decode("utf-8"), contents)

if __name__ == '__main__':
    unittest.main()
