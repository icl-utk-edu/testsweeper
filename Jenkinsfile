pipeline {
    agent none
    triggers { cron ('H H(0-2) * * *') }
    stages {
        //======================================================================
        stage('Parallel Build') {
            matrix {
                axes {
                    axis {
                        name 'maker'
                        values 'make', 'cmake'
                    }
                } // axes
                stages {
                //--------------------------------------------------------------
                stage('Build - Caffeine gcc') {
                    agent { node 'caffeine.icl.utk.edu' }
                    steps {
                        sh '''
                        #!/bin/sh +x
                        echo "TestSweeper Building"
                        hostname && pwd

                        source /home/jmfinney/spack/share/spack/setup-env.sh
                        spack load gcc@6.4.0

                        echo "maker ${maker}"
                        if [ "${maker}" = "make" ]; then
                            export color=no
                            make distclean
                            make config CXXFLAGS="-Werror"
                            make -j8
                            ldd example
                            cd test
                            ./run_tests.py --xml ../report_make.xml
                        fi
                        if [ "${maker}" = "cmake" ]; then
                            spack load cmake
                            rm -rf build
                            mkdir build
                            cd build
                            cmake -DCOLOR=no -DCMAKE_CXX_FLAGS="-Werror" ..
                            make -j8
                            ldd example
                            cd test
                            ./run_tests.py --xml ../../report_cmake.xml
                        fi
                        '''
                    } // steps
                    post {
                        unstable {
                            slackSend channel: '#slate_ci',
                                color: 'warning',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} unstable (<${env.BUILD_URL}|Open>)"
                        }
                        failure {
                            slackSend channel: '#slate_ci',
                                color: 'danger',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} failed (<${env.BUILD_URL}|Open>)"
                            mail to: 'slate-dev@icl.utk.edu',
                                subject: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} failed",
                                body: "See more at ${env.BUILD_URL}"
                        }
                        always {
                            junit '*.xml'
                        }
                    } // post
                } // stage(Build - Caffeine)

                //--------------------------------------------------------------
                stage('Build - Lips gcc') {
                    agent { node 'lips.icl.utk.edu' }
                    steps {
                        sh '''
                        #!/bin/sh +x
                        echo "TestSweeper Building"
                        hostname && pwd

                        source /home/jmfinney/spack/share/spack/setup-env.sh
                        spack load gcc@6.4.0

                        echo "maker ${maker}"
                        if [ "${maker}" = "make" ]; then
                            export color=no
                            make distclean
                            make config CXXFLAGS="-Werror"
                            make -j8
                            ldd example
                            cd test
                            ./run_tests.py --xml ../report_make.xml
                        fi
                        if [ "${maker}" = "cmake" ]; then
                            spack load cmake
                            rm -rf build
                            mkdir build
                            cd build
                            cmake -DCOLOR=no -DCMAKE_CXX_FLAGS="-Werror" ..
                            make -j8
                            ldd example
                            cd test
                            ./run_tests.py --xml ../../report_cmake.xml
                        fi
                        '''
                    } // steps
                    post {
                        unstable {
                            slackSend channel: '#slate_ci',
                                color: 'warning',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} unstable (<${env.BUILD_URL}|Open>)"
                        }
                        failure {
                            slackSend channel: '#slate_ci',
                                color: 'danger',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} failed (<${env.BUILD_URL}|Open>)"
                            mail to: 'slate-dev@icl.utk.edu',
                                subject: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} failed",
                                body: "See more at ${env.BUILD_URL}"
                        }
                        always {
                            junit '*.xml'
                        }
                    } // post
                } // stage(Build - Lips)
                } // stages
            } // matrix
        } // stage(Parallel Build)
    } // stages
} // pipeline
