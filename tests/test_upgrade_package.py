from pathlib import Path
import hashlib,json,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from upgrade_badge import verify_package

class PackageTests(unittest.TestCase):
    def test_hash_missing_and_modified(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);data=b'example';file=root/'app.bin';file.write_bytes(data)
            (root/'release.json').write_text(json.dumps({'version':'test','sha256':{'app.bin':hashlib.sha256(data).hexdigest()}}))
            self.assertEqual(verify_package(root),'test')
            file.write_bytes(b'modified')
            with self.assertRaisesRegex(ValueError,'modified'):verify_package(root)
            file.unlink()
            with self.assertRaises(OSError):verify_package(root)
    def test_escape_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'release.json').write_text(json.dumps({'version':'test','sha256':{'../outside':'0'*64}}))
            with self.assertRaisesRegex(ValueError,'modified'):verify_package(root)

if __name__=='__main__':unittest.main()
