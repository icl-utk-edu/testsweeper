pipeline {

agent none
options {
    // Required to clean before build
    skipDefaultCheckout( true )
}
triggers { pollSCM 'H/10 * * * *' }
stages {
    //======================================================================
    stage('Parallel Build') {
        matrix {
            axes {
                axis {
                    name 'maker'
                    values 'make', 'cmake'
                }
                axis {
                    name 'host'
                    values 'cpu_intel'
                }
            } // axes
            stages {
                stage('Build') {
                    agent { label "${host}" }

                    //----------------------------------------------------------
                    steps {
                        cleanWs()
                        checkout scm
                        sh '''
#!/bin/sh

set +e  # errors are not fatal (e.g., Spack sometimes has spurious failures)
set -x  # echo commands

date
hostname && pwd
export top=`pwd`

# Suppress echo (-x) output of commands executed with `run`. Useful for Spack.
# set +x, set -x are not echo'd.
run() {
    { set +x; } 2> /dev/null;
    $@;
    set -x
}

# Suppress echo (-x) output of `print` commands. https://superuser.com/a/1141026
# aliasing `echo` causes issues with spack_setup, so use `print` instead.
echo_and_restore() {
    builtin echo "$*"
    case "$save_flags" in
        (*x*)  set -x
    esac
}
alias print='{ save_flags="$-"; set +x; } 2> /dev/null; echo_and_restore'

print "======================================== load compiler"
date
run source /home/jenkins/spack_setup
run sload gcc@7.3.0
run spack compiler find

print "======================================== verify spack"
# Check what is loaded.
run spack find --loaded

which g++
g++ --version

print "======================================== env"
env

print "======================================== setup build"
date
print "maker ${maker}"
export color=no
rm -rf ${top}/install
if [ "${maker}" = "make" ]; then
    make distclean
    make config CXXFLAGS="-Werror" prefix=${top}/install
fi
if [ "${maker}" = "cmake" ]; then
    run sload cmake
    which cmake
    cmake --version

    rm -rf build && mkdir build && cd build
    cmake -Dcolor=no -DCMAKE_CXX_FLAGS="-Werror" \
          -DCMAKE_INSTALL_PREFIX=${top}/install ..
fi

print "======================================== build"
date
make -j8

print "======================================== install"
date
make -j8 install
ls -R ${top}/install

print "======================================== verify build"
date
ldd test/tester

print "======================================== tests"
print "Run tests."
date
cd test
./run_tests.py --xml ${top}/report-${maker}.xml

# todo smoke tests

date
'''
                    } // steps

                    //----------------------------------------------------------
                    post {
                        failure {
                            mail to: 'slate-dev@icl.utk.edu',
                                subject: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} ${host} failed",
                                body: "See more at ${env.BUILD_URL}"
                        }
                        always {
                            junit '*.xml'
                        }
                    } // post

                } // stage(Build)
            } // stages
        } // matrix
    } // stage(Parallel Build)
} // stages

} // pipeline
