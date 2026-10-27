from pathlib import Path
import unittest
import xml.etree.ElementTree as E
ROOT=Path(__file__).resolve().parents[1]
class SourceLayoutTests(unittest.TestCase):
 def test_plain_project_and_icon_directories(self):
  for name in ['Codecs','Core','Modern','Project','Specification','UI_Modern','Universal','ZS_Codecs','Package','icons']:
   with self.subTest(folder=name): self.assertTrue((ROOT/name).is_dir(),name)
  self.assertFalse(any(p.is_dir() and p.name.startswith('ExcellentZip') for p in ROOT.iterdir()))
 def test_solution_project_paths_exist(self):
  for p in E.parse(ROOT/'ExcellentZip.slnx').iter('Project'):
   self.assertTrue((ROOT/p.get('Path')).is_file(),p.get('Path'))
if __name__=='__main__': unittest.main()
