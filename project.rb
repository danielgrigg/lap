# This package isn't used, it's merely to show how packages are declared.
PACKAGE_BOOST = {
  :name => "Boost",
  :components => "system filesystem",
#  :version => "1.36.0",
  :required => true,
  :optional_cmake => ""  # Insert package-missing-handler
};
PACKAGE_OPENEXR = {
  :name => "OpenEXR",
#  :components => "",
  :required => true,
  :optional_cmake => ""  # Insert package-missing-handler
}
PROJECT = 
{
  :name => "lap",
  :cmake_version => "2.6",

  :targets => 
  [{
    :name => "lap",
    :type => :static, 
    :install => true, 
    :sources => "src",
    :common => 
    {
      :packages => [PACKAGE_BOOST],
      :definitions => [],
      :include_dirs => ["src"],
      :link_dirs => [],
      :libs => []
    },
    :apple => 
    {
      :packages => [],
      :definitions => [],
      :include_dirs => [],
      :link_dirs => [] 
    },
    :linux => 
    {
      :packages => [],
      :definitions => ["IS_LINUX"],
      :include_dirs => [],
      :link_dirs => [] 
    },
    :windows => 
    {
      :packages => [],
      :definitions => ["IS_WINDOWS"],
      :include_dirs => [],
      :link_dirs => [] 
    }
  }, 
  {
    :name => "objdump",
    :type => :executable,
    :depends => "lap",
    :install => false,
    :sources => "apps/objdump",
    :common => 
    {
      :packages => [],
      :definitions => [],
      :include_dirs => [],
      :link_dirs => [],
      :libs => ["lap"]
    }
  },
  {
    :name => "meshdump",
    :type => :executable,
    :depends => "lap",
    :install => false,
    :sources => "apps/meshdump",
    :common => 
    {
      :packages => [],
      :definitions => [],
      :include_dirs => [],
      :link_dirs => [],
      :libs => ["lap"]
    }
  },
  {
    :name => "lapinfo",
    :type => :executable,
    :depends => "lap",
    :install => true,
    :sources => "apps/lapinfo",
    :common => 
  {
    :packages => [],
    :definitions => [],
    :include_dirs => [],
    :link_dirs => [],
    :libs => ["lap"]
  }
  },
  {
    :name => "mesh2obj",
    :type => :executable,
    :depends => "lap",
    :install => false,
    :sources => "tests/mesh2obj",
    :common => 
  {
    :packages => [],
    :definitions => [],
    :include_dirs => [],
    :link_dirs => [],
    :libs => ["lap"]
  }
  }, {
    :name => "lapquery",
    :type => :executable,
    :depends => "lap",
    :install => true,
    :sources => "apps/lapquery",
    :common => 
    {
      :packages => [],
      :definitions => [],
      :include_dirs => [],
      :link_dirs => [],
      :libs => ["lap"]
    }}
  ]
}

