BENCHMARKS_HOME=/data/qinjingyuan/benchmarks

apps=()
bins=()
args=()


base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/500.perlbench_r/run/run_base_refrate_prefetch-m64.0000
apps+=("perlbench_r_base.prefetch-m64")
bins+=("${base_dir}/perlbench_r_base.prefetch-m64")
args+=("${base_dir}/diffmail.pl 4 800 10 17 19 300 -I${base_dir}/lib")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/502.gcc_r/run/run_base_refrate_prefetch-m64.0000
apps+=("cpugcc_r_base.prefetch-m64")
bins+=("${base_dir}/cpugcc_r_base.prefetch-m64")
args+=("${base_dir}/gcc-smaller.c -O3 -fipa-pta -o ${base_dir}/gcc-smaller.opts-O3_-fipa-pta.s")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/505.mcf_r/run/run_base_refrate_prefetch-m64.0000
apps+=("mcf_r_base.prefetch-m64")
bins+=("${base_dir}/mcf_r_base.prefetch-m64")
args+=("${base_dir}/inp.in")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/523.xalancbmk_r/run/run_base_refrate_prefetch-m64.0000
apps+=("cpuxalan_r_base.prefetch-m64")
bins+=("${base_dir}/cpuxalan_r_base.prefetch-m64")
args+=("-v ${base_dir}/t5.xml ${base_dir}/xalanc.xsl")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/525.x264_r/run/run_base_refrate_prefetch-m64.0000
apps+=("ldecod_r_base.prefetch-m64")
bins+=("${base_dir}/ldecod_r_base.prefetch-m64")
args+=("-i ${base_dir}/BuckBunny.264 -o ${base_dir}/BuckBunny.yuv")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/531.deepsjeng_r/run/run_base_refrate_prefetch-m64.0000
apps+=("deepsjeng_r_base.prefetch-m64")
bins+=("${base_dir}/deepsjeng_r_base.prefetch-m64")
args+=("${base_dir}/ref.txt")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/541.leela_r/run/run_base_refrate_prefetch-m64.0000
apps+=("leela_r_base.prefetch-m64 ")
bins+=("${base_dir}/leela_r_base.prefetch-m64 ")
args+=("${base_dir}/ref.sgf")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/548.exchange2_r/run/run_base_refrate_prefetch-m64.0000
apps+=("exchange2_r_base.prefetch-m64")
bins+=("${base_dir}/exchange2_r_base.prefetch-m64")
args+=("6")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/557.xz_r/run/run_base_refrate_prefetch-m64.0000
apps+=("xz_r_base.prefetch-m64")
bins+=("${base_dir}/xz_r_base.prefetch-m64")
args+=("${base_dir}/cld.tar.xz  160 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 59796407 61004416 6")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/503.bwaves_r/run/run_base_refrate_prefetch-m64.0000
apps+=("bwaves_r_base.prefetch-m64 bwaves_1")
bins+=("${base_dir}/bwaves_r_base.prefetch-m64 bwaves_1")
args+=("bwaves_1 --input=${base_dir}/bwaves_1.in")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/507.cactuBSSN_r/run/run_base_refrate_prefetch-m64.0000
apps+=("cactusBSSN_r_base.prefetch-m64")
bins+=("${base_dir}/cactusBSSN_r_base.prefetch-m64")
args+=("${base_dir}/spec_ref.par")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/508.namd_r/run/run_base_refrate_prefetch-m64.0000
apps+=("namd_r_base.prefetch-m64")
bins+=("${base_dir}/namd_r_base.prefetch-m64")
args+=("--input ${base_dir}/apoa1.input --output ${base_dir}/apoa1.ref.output --iterations 65")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/511.povray_r/run/run_base_refrate_prefetch-m64.0000
apps+=("povray_r_base.prefetch-m64")
bins+=("${base_dir}/povray_r_base.prefetch-m64")
args+=("${base_dir}/SPEC-benchmark-ref.ini")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/519.lbm_r/run/run_base_refrate_prefetch-m64.0000
apps+=("lbm_r_base.prefetch-m64")
bins+=("${base_dir}/lbm_r_base.prefetch-m64")
args+=("3000 ${base_dir}/reference.dat 0 0 ${base_dir}/100_100_130_ldc.of")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/526.blender_r/run/run_base_refrate_prefetch-m64.0000
apps+=("blender_r_base.prefetch-m64")
bins+=("${base_dir}/blender_r_base.prefetch-m64")
args+=("${base_dir}/sh3_no_char.blend --render-output ${base_dir}/sh3_no_char_ --threads 1 -b -F RAWTGA -s 849 -e 849 -a")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/527.cam4_r/run/run_base_refrate_prefetch-m64.0000
apps+=("cam4_r_base.prefetch-m64")
bins+=("${base_dir}/cam4_r_base.prefetch-m64")
args+=("")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/538.imagick_r/run/run_base_refrate_prefetch-m64.0000
apps+=("imagick_r_base.prefetch-m64")
bins+=("${base_dir}/imagick_r_base.prefetch-m64")
args+=("-limit disk 0 ${base_dir}/refrate_input.tga -edge 41 -resample 181% -emboss 31 -colorspace YUV -mean-shift 19x19+15% -resize 30% ${base_dir}/refrate_output.tga")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/544.nab_r/run/run_base_refrate_prefetch-m64.0000
apps+=("nab_r_base.prefetch-m64")
bins+=("${base_dir}/nab_r_base.prefetch-m64")
args+=("${base_dir}/1am0 1122214447 122")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/549.fotonik3d_r/run/run_base_refrate_prefetch-m64.0000
apps+=("fotonik3d_r_base.prefetch-m64")
bins+=("${base_dir}/fotonik3d_r_base.prefetch-m64")
args+=("")

base_dir=$BENCHMARKS_HOME/spec2017/benchspec/CPU/554.roms_r/run/run_base_refrate_prefetch-m64.0000
apps+=("roms_r_base.prefetch-m64")
bins+=("${base_dir}/roms_r_base.prefetch-m64")
args+=("--input=${base_dir}/ocean_benchmark2.in.x")

mpr_dir=/data/lixiang/mem-pattern-recognition

for((i=0;i<20;i++))
do
    ${mpr_dir}/scripts/analyze_roi.sh ${apps[$i]} ${bins[$i]} "${args[$i]}" &
done
  
    