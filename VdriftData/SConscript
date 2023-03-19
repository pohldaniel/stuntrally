import os, stat

#-------------#
# Import Vars #
#-------------#
Import('*')

#---------#
# Sources #
#---------#
src = []

for root, dirs, files in os.walk("."):
  if root.find('.svn') == -1:
    for file in [f for f in files if not f.endswith('~')]:
      src.append(os.path.join(root, file))
      install = env.Install(Dir(env.subst('$data_directory/'+root)), os.path.join(root, file))

#---------------------------------#
# Distribute to src_dir & bin_dir #
#---------------------------------#
#dist_files = ['SConscript'] + src

#env.Distribute (src_dir, dist_files)
#env.Distribute (bin_dir, src)

#Export(['env', 'src_dir', 'bin_dir'])

env.Alias('install', install)

# Install locale
import glob
src = glob.glob('locale/*')
po_files = [os.path.basename(po) for po in src if po.endswith('.po')]
languages = [po[0:-3] for po in po_files]
for lang in languages:
    mo_tgt_file = lang + '/LC_MESSAGES/vdrift.mo'
    mo_file = 'locale/' + mo_tgt_file
    po_file = 'locale/' + lang + ".po"
    env.MoBuild(mo_file, po_file)
    install = env.InstallAs(os.path.join(env.subst('$locale_directory'), mo_tgt_file), mo_file)
    env.Alias("install", install)
