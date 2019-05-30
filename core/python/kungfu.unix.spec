block_cipher = None
a = Analysis(['kungfu/__main__.py'],
     pathex=['python'],
     binaries=[],
     datas=[
        ('../build/' + os.environ['CMAKEBUILDTYPE'] + '/*', '.'),
        ('extensions', 'extensions'),
        ('../build/build_extensions', 'extensions'),
        ('../cpp/yijinjing/include', 'include'),
        ('../cpp/wingchun/include', 'include'),
        ('../build/deps/nanomsg-1.1.5/include', 'include'),
        ('../deps/spdlog-1.3.1/include', 'include'),
        ('../deps/json-3.5.0/single_include', 'include'),
        ('../deps/fmt-5.3.0/include', 'include'),
        ('../deps/hffix-b67d404f/include', 'include'),
        ('../deps/SQLiteCpp-2.3.0/include', 'include')
     ],
     hiddenimports=[
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
