import re
import os, shutil
import tarfile
import extensions
import kungfu.extension as kfext

@kfext.arg("-f", "--file", dest="file", required=True, help="KungFu extension file (tgz)")
@kfext.command(help='install extension')
def install(args, logger):
    filename = os.path.basename(args.file)
    filename_re = re.match(r'(\w+)-v\d+\.\d+\.\d+\.tgz', filename)
    if filename_re:
        ext_name = filename_re.group(1)
        install_path = os.path.join(extensions.extension_path, ext_name)
        ext_cache_dir = os.path.join(extensions.extension_path, '.cache')
        os.makedirs(ext_cache_dir, exist_ok=True)
        extfile = tarfile.open(args.file)
        if not extfile.getmember('package'):
            print('Bad extension file')
            logger.error('Bad extension file')
        else:
            logger.info('extracting extension file %s to %s', args.file, ext_cache_dir)
            extfile.extractall(ext_cache_dir)
            if os.path.exists(install_path):
                logger.info('Overwriting extension %s', ext_name)
                shutil.rmtree(install_path)
            os.rename(os.path.join(ext_cache_dir, 'package'), install_path)
            logger.info("extension %s installed", ext_name)
            print('Installed extension', ext_name)
    else:
        print('Bad extension filename', filename)
        logger.error("Invalid extension filename %s", args.file)