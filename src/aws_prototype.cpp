#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/glacier/GlacierClient.h>
#include <aws/glacier/model/DescribeVaultRequest.h>

#include <iostream>

int main()
{
    Aws::SDKOptions options;
    
    Aws::InitAPI(options);

    Aws::Client::ClientConfiguration client_cfg;
    client_cfg.region = Aws::Region::EU_CENTRAL_1;
    Aws::Auth::EnvironmentAWSCredentialsProvider cred_provider;
    Aws::Auth::AWSCredentials creds = cred_provider.GetAWSCredentials();
    Aws::Glacier::GlacierClient client(client_cfg);
    Aws::Glacier::Model::DescribeVaultRequest request;
    request.WithVaultName("TestVault").WithAccountId("-");
    Aws::Glacier::Model::DescribeVaultOutcome outcome = client.DescribeVault(request);
    if(!outcome.IsSuccess()) {
        std::cout << ":(" << std::endl;
        std::cout << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << ":)" << std::endl;
        auto const& res = outcome.GetResult();
        std::cout << "Vault is " << res.GetVaultARN() << " '" << res.GetVaultName() << "'\n"
                  << "    created at " << res.GetCreationDate() << "\n"
                  << "    last inventory at " << res.GetLastInventoryDate() << "\n"
                  << "    " << res.GetNumberOfArchives() << " archives with a total of "
                            << res.GetSizeInBytes() << " bytes."
                  << std::endl;

    }

    Aws::ShutdownAPI(options);
}

