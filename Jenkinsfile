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
                axis {
                    name 'host'
                    values 'caffeine', 'lips'
                }
            } // axes
            stages {
                stage('Build') {
                    agent { node "${host}.icl.utk.edu" }

                    //----------------------------------------------------------
                    steps {
                        sh '''
                        #!/bin/sh +x
                        hostname && pwd

                        source /home/jenkins/spack_setup
                        sload gcc@6.4.0

                        echo "========================================"
                        echo "maker ${maker}"
                        if [ "${maker}" = "make" ]; then
                            export color=no
                            make distclean
                            make config CXXFLAGS="-Werror"
                            export top=..
                        fi
                        if [ "${maker}" = "cmake" ]; then
                            sload cmake
                            rm -rf build
                            mkdir build
                            cd build
                            cmake -Dcolor=no -DCMAKE_CXX_FLAGS="-Werror" ..
                            export top=../..
                        fi

                        echo "========================================"
                        make -j8

                        echo "========================================"
                        ldd test/tester

                        echo "========================================"
                        cd test
                        ./run_tests.py --xml ${top}/report-${maker}.xml
                        '''
                    } // steps

                    //----------------------------------------------------------
                    post {
                        changed {
                            slackSend channel: '#slate_ci',
                                color: 'good',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} ${host} changed (<${env.BUILD_URL}|Open>)"
                        }
                        unstable {
                            slackSend channel: '#slate_ci',
                                color: 'warning',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} ${host} unstable (<${env.BUILD_URL}|Open>)"
                        }
                        failure {
                            slackSend channel: '#slate_ci',
                                color: 'danger',
                                message: "${currentBuild.fullDisplayName} >> ${STAGE_NAME} >> ${maker} ${host} failed (<${env.BUILD_URL}|Open>)"
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
