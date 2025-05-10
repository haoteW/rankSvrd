#!/bin/bash

set -e  # 出错即退出
set -o pipefail

echo "start build hiredis 和 redis-plus-plus ..."

# 安装编译工具和依赖
echo "📦 安装依赖包..."
sudo apt-get update
sudo apt-get install -y cmake g++ make libssl-dev git

# 设置工作目录
WORKDIR=$HOME/redis_build_env
mkdir -p "$WORKDIR"
cd "$WORKDIR"

# 克隆并构建 hiredis
echo "🔧 构建 hiredis ..."
if [ ! -d "hiredis" ]; then
    git clone git@github.com:redis/hiredis.git
fi

cd hiredis
make -j
sudo make install
cd ..

# 克隆并构建 redis-plus-plus
echo "🔧 构建 redis-plus-plus ..."
if [ ! -d "redis-plus-plus" ]; then
    git clone git@github.com:sewenew/redis-plus-plus.git
fi

cd redis-plus-plus
mkdir -p build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DREDIS_PLUS_PLUS_BUILD_STATIC=ON \
  -DREDIS_PLUS_PLUS_BUILD_SHARED=ON \
  -DREDIS_PLUS_PLUS_BUILD_TEST=OFF

make -j
sudo make install

echo "✅ 构建完成，hiredis 与 redis-plus-plus 已安装！"

