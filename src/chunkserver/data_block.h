// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  BAIDU_BFS_DATA_BLOCK_H_
#define  BAIDU_BFS_DATA_BLOCK_H_

#include <string>
#include <vector>

#include <common/mutex.h>
#include <common/thread_pool.h>
#include <common/sliding_window.h>

namespace baidu {
namespace bfs {

struct Buffer {
    const char* data_;
    int32_t len_;
    Buffer(const char* buff, int32_t len)
      : data_(buff), len_(len) {}
    Buffer()
      : data_(NULL), len_(0) {}
    Buffer(const Buffer& o)
      : data_(o.data_), len_(o.len_) {}
};

/// Meta of a data block
struct BlockMeta {
    int64_t block_id;
    int64_t block_size;
    int64_t checksum;
    int64_t version;
    BlockMeta()
      : block_id(0), block_size(0), checksum(0), version(-1) {
    }
};

class FileCache;

/// Data block
class Block {
public:
    Block(const BlockMeta& meta, const std::string& store_path, ThreadPool* thread_pool,
          FileCache* file_cache);
    ~Block();
    /// Getter
    int64_t Id() const;
    int64_t Size() const;
    std::string GetFilePath() const;
    BlockMeta GetMeta() const;
    int64_t DiskUsed();
    bool SetDeleted();
    void SetVersion(int64_t version);
    int GetVersion();
    /// Open corresponding file for write.
    bool OpenForWrite();
    /// Set expected slice num, for IsComplete.
    void SetSliceNum(int32_t num);
    /// Is all slice is arrival(Notify by the sliding window)
    bool IsComplete();
    /// Read operation.
    int32_t Read(char* buf, int32_t len, int64_t offset);
    /// Write operation.
    bool Write(int32_t seq, int64_t offset, const char* data,
               int32_t len, int64_t* add_use = NULL);
    /// Flush block to disk.
    bool Close();
    void AddRef();
    void DecRef();
private:
    /// Invoke by slidingwindow, when next buffer arrive.
    void WriteCallback(int32_t seq, Buffer buffer);
    void DiskWrite();
    /// Append to block buffer
    void Append(int32_t seq, const char*buf, int32_t len);
private:
    enum Type {
        InDisk,
        InMem,
    };
    ThreadPool* thread_pool_;
    BlockMeta   meta_;
    int32_t     last_seq_;
    int32_t     slice_num_;
    char*       blockbuf_;
    int32_t     buflen_;
    int32_t     bufdatalen_;
    std::vector<std::pair<const char*,int> > block_buf_list_;
    bool        disk_writing_;
    std::string disk_file_;
    int32_t     disk_file_size_;
    int         file_desc_; ///< disk file fd
    volatile int refs_;
    Mutex       mu_;
    common::SlidingWindow<Buffer>* recv_window_;
    bool        finished_;
    volatile int deleted_;

    FileCache*  file_cache_;
};

}
}

#endif  // BAIDU_BFS_BLOCK_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */