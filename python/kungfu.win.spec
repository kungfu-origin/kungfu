block_cipher = None
a = Analysis(['kungfu\\__main__.py'],
     pathex=['python'],
     binaries=[('..\\build\\' + os.environ['CMAKEBUILDTYPE'] + '\\*.dll', '.'),
               ('..\\build\\' + os.environ['CMAKEBUILDTYPE'] + '\\py*.pyd', '.'),
               ('..\\build\\cpp\\deps\\nnpy-1.4.2\\build\\lib*\\*.pyd', '.'),
               ('C:\\local\\boost_1_64_0\\lib64-msvc-14.1\\*.dll', '.')
               ],
     datas=[
          ('..\\build\\build_extensions', 'extensions'),
          ('..\\build\\' + os.environ['CMAKEBUILDTYPE'] + '\\*xtp*.pyd', 'extensions\\xtp'),
          ('..\\build\\' + os.environ['CMAKEBUILDTYPE'] + '\\*ctp*.pyd', 'extensions\\ctp'),
          ('..\\build\\build_extensions', 'extensions'),
          ('extensions\\*', 'extensions')
     ],
     hiddenimports=[
          'cffi',
          'numpy',
          'pandas'
          ],
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