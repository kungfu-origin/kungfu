block_cipher = None
a = Analysis(['kungfu\\KungFu.py'],
     pathex=['python'],
     binaries=[('..\\build\\' + os.environ['CMAKEBUILDTYPE'] + '\\*', '.'),
               ('..\\build\\cpp\\yijinjing\\pybind\\' + os.environ['CMAKEBUILDTYPE'] + '\\*.pyd', '.'),
               ('..\\build\\cpp\\deps\\nnpy-1.4.2\\build\\lib*\\*.pyd', '.'),
               ('C:\\local\\boost_1_64_0\\lib64-msvc-14.1\\*.dll', '.')
               ],
     datas=None,
     hiddenimports=[],
     hookspath=None,
     runtime_hooks=None,
     excludes=None,
     cipher=block_cipher)
pyz = PYZ(a.pure, a.zipped_data,
     cipher=block_cipher)
exe = EXE(pyz,
                a.scripts,
                exclude_binaries=True,
                name='kfc',
                debug=False,
                strip=False,
                upx=True,
                console=True )
coll = COLLECT(exe,
                    a.binaries,
                    a.zipfiles,
                    a.datas,
                    strip=False,
                    upx=True,
                    name='kfc')