export const VCDepsWinRegMap: Record<KungfuApi.VCDepsVersionTypes, string[]> = {
  '2008': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\67D6ECF5CD5FBA732B8B22BAC8DE1B4D',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\6F9E66FF7E38E3A3FA41D89E8A906A4A',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}',
  ],
  '2010': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\1926E8D15D0BCE53481466615F760A7F',
  ],
  '2012': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}',
  ],
  '2013': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{050d4fc8-5d48-4b8f-8972-47c82c46020f}',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{050d4fc8-5d48-4b8f-8972-47c82c46020f}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{f65db027-aff3-4070-886a-0d87064aabb1}',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{f65db027-aff3-4070-886a-0d87064aabb1}',
  ],
  '2015-2019': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x64,amd64,14.32,bundle',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{2d507699-404c-4c8b-a54a-38e352f32cdd}',
  ],
  '2015-2022': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x86,x86,14.29,bundle',
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{8d5fdf81-7022-423f-bd8b-b513a1050ae1}',
  ],
};

/*
Microsoft Visual C++ 2008 Redistributable - x64 9.0.30729.6161
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Products\67D6ECF5CD5FBA732B8B22BAC8DE1B4D

Microsoft Visual C++ 2008 Redistributable - x86 9.0.21022
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Products\6F9E66FF7E38E3A3FA41D89E8A906A4A
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}

Microsoft Visual C++ 2010  x64 Redistributable - 10.0.40219
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Products\1926E8D15D0BCE53481466615F760A7F

2010 x86 还没有找到安装路径

Microsoft Visual C++ 2012 Redistributable (x64) - 11.0.61030
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}

Microsoft Visual C++ 2012 Redistributable (x86) - 11.0.61030
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}

Microsoft Visual C++ 2013 Redistributable (x64) - 12.0.30501
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\{050d4fc8-5d48-4b8f-8972-47c82c46020f}
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{050d4fc8-5d48-4b8f-8972-47c82c46020f}

Microsoft Visual C++ 2013 Redistributable (x86) - 12.0.30501
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\{f65db027-aff3-4070-886a-0d87064aabb1}
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{f65db027-aff3-4070-886a-0d87064aabb1}

Microsoft Visual C++ 2015-2022 Redistributable (x64) - 14.32.31326
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\VC,redist.x64,amd64,14.32,bundle
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{2d507699-404c-4c8b-a54a-38e352f32cdd}

Microsoft Visual C++ 2015-2019 Redistributable (x86) - 14.29.30139
HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\VC,redist.x86,x86,14.29,bundle
HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{8d5fdf81-7022-423f-bd8b-b513a1050ae1}
 */
