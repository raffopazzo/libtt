script_dir=$(dirname $0)
docker build \
    -f ${script_dir}/devbox.Dockerfile \
    --rm --tag libtt-devbox \
    ${script_dir}
