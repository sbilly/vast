#include "vast/archive.h"

#include "vast/exception.h"
#include "vast/file_system.h"
#include "vast/logger.h"
#include "vast/segment.h"
#include "vast/segment_manager.h"

using namespace cppa;

namespace vast {

archive::archive(std::string const& directory, size_t max_segments)
  : directory_(directory)
{
  segment_manager_ = spawn<segment_manager>(max_segments, directory_);
}

void archive::init()
{
  VAST_LOG_ACT_VERBOSE("archive", "spawned");
  become(
      on(atom("init")) >> [=]
      {
        path p(directory_);
        if (! exists(p))
        {
          VAST_LOG_ACT_INFO("archive", "creates new directory " << directory_);

          if (! mkdir(p))
          {
            VAST_LOG_ACT_ERROR("archive", " failed to create directory " <<
                               directory_);
            quit();
          }
        }
        segment_manager_ << last_dequeued();
      },
      on(atom("kill")) >> [=]()
      {
        segment_manager_ << last_dequeued();
        quit();
      },
      on(atom("get"), atom("ids")) >> [=]
      {
        forward_to(segment_manager_);
      },
      on(atom("get"), arg_match) >> [=](uuid const& /* id */)
      {
        forward_to(segment_manager_);
      },
      on(arg_match) >> [=](segment const& /* s */)
      {
        forward_to(segment_manager_);
      });
}

void archive::on_exit()
{
  VAST_LOG_ACT_VERBOSE("archive", "terminated");
}


} // namespace vast
