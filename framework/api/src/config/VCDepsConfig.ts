export const VCDepsWinRegMap: Record<KungfuApi.VCDepsVersionTypes, string[]> = {
  '2008': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\67D6ECF5CD5FBA732B8B22BAC8DE1B4D', // x64
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{5FCE6D76-F5DC-37AB-B2B8-22AB8CEDB1D4}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\6F9E66FF7E38E3A3FA41D89E8A906A4A', // x86
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}',
  ],
  '2010': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\1926E8D15D0BCE53481466615F760A7F', // x64
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{1D8E6291-B0D5-35EC-8441-6616F567A0F7}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Products\\1D5E3C0FEDA1E123187686FED06E995A', // x86
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}',
  ],
  '2012': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}', //x64
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{ca67548a-5ebe-413a-b50c-4b9ceb6d66c6}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}', //x86
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}',
  ],
  '2013': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{042d26ef-3dbe-4c25-95d3-4c1b11b235a7}', // x64
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{042d26ef-3dbe-4c25-95d3-4c1b11b235a7}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\{f65db027-aff3-4070-886a-0d87064aabb1}', // x86
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{f65db027-aff3-4070-886a-0d87064aabb1}',
  ],
  '2015-2022': [
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x64,amd64,14.34,bundle', //x64
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{d92971ab-f030-43c8-8545-c66c818d0e05}',
    'HKLM\\SOFTWARE\\Classes\\Installer\\Dependencies\\VC,redist.x86,x86,14.34,bundle', //x86
    'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{6ba9fb5e-8366-4cc4-bf65-25fe9819b2fc}',
  ],
};
