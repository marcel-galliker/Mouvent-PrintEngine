import unittest, os, subprocess, time, glob


class ArtefactTests(unittest.TestCase):


    def test_git(self):
        "Test there is no modified file after compilation"
        git = subprocess.run(["git","status","--porcelain"], capture_output=True, text=True )
        self.assertEqual(git.returncode, 0, "Git did not return correctly")
        self.assertEqual(len(git.stdout), 0, "Modified files exist:\n"+git.stdout)
  
    def test_bin(self):
        "Test bin artefact"
        created = glob.glob("bin/**", recursive=True)
        with open("test/binfiles.txt") as binfiles:
            needed = binfiles.read().split('\n')
        self.maxDiff = None
        self.assertCountEqual(created, needed, "Incorect build files in bin\nPlease modify test/binfiles.txt according to bin artefact") 
            


if __name__ == '__main__':
    unittest.main()