# Group 02 

![pipeline_status](https://git.chalmers.se/courses/dit638/students/2023-group-02/badges/main/pipeline.svg)

# Instructions

**Requirements**

```sh
git >= 2.0.0
docker >= 1.12.0
SSH key pair, with the public key added to Chalmers Gitlab 
```
## How to clone?
Create a directory, open the terminal, navigate to the newly created directory and run:
```sh
git clone git@git.chalmers.se:courses/dit638/students/2023-group-02.git
```

## How to build?

Open terminal and run the command in the 2023-group-02 repo folder. 
```sh
docker build -t CID/example:latest -f Dockerfile .
```
If the build process is successful, you can run the resulting image inside a container by running the following command:
```sh
docker run --rm CID/example:latest 42
```

# How to work with Git and GitLab

## How to make a commit?

Make sure to add your email and user name first by running the below commands 
```sh
git config --global user.email "you@example.com"
git config --global user.name “your name”
```
Make sure you are on the right branch and that your code is functional then run

```sh
git status //check the current staged files
git add . //stage all changed files
git pull //make sure the local branch is up to date with the remote
git commit -m “your message” -m “your comment” 
git push // push to remote branch 
```
Note: You should inform people to pull your changes.



## How to write meaningful commit messages?
- Keep messages short and descriptive 
- Refer to the issue by including it at the beginning of the commit message
- Add comments to make additional clarifications 

Note: refer to the **commit message structure** section

## How to work on a feature?
- See open issues / create an issue 
- Create a remote branch for that feature/issue and check out to it locally 
- Create tests (when applicable)
- Make small and well documented changes, multiple commits are encouraged instead of multiple changes in single commit 
- Do not push not working code to the remote branch before opening a merge request  
- Make sure to update the branch often in case the root branch changes
- Make sure the tests pass on the branch before creating a PR
- Create a PR as soon as the tests pass and the code is functional and complete (opt remind people to review your PR)
- Make necessary changes in case you receive feedback 
- To change between branches, you need to make sure all changes on current branch are committed or stashed 
- Close the issue if the Definition of Done is met / open new branch to continue solving the issue 


## How to create a merge request?
**Rules:**
- Merge requests should only be made if they intend to close an issue or multiple issues. 
- They should contain the keyword “Closes” and the issue number next to it (above the merge request description) 
- Merge requests should have a description that includes what features/issues were implemented, how and why.
- Merge requests should be approved by a team member in order for the merge to happen. 

**Instructions**
- Make sure the local branch is up to date with the remote   
- Make sure your latest changes are pushed 
- Make sure the tests are passing
- Make sure the branch is updated with the latest root 
- Checkout locally to the desired branch you want to merge with: 
```sh
git checkout <name of branch to be merged into>
git pull
git merge <name of the branch to be merged> 
```

# Commit message structure

**Subject** - This should be a short, specific and meaningful “title” for the commit that doesn’t contain a period at the end and starts like a regular title(i.e with an Uppercase letter). Titles that are vague and ambiguous are not acceptable(i.e various fixes).

Rules:
- Describe why a change is being made.
- Limit the subject line to 50 characters
- Use the imperative mood in the subject line
- Refer to issue:  #issueNumber
- Do not end the subject line with a period
- The first word should be the type (imperative)

Types:
- **fix**: use when patching a bug 
- **close #issueNumber**:  automatically close the issue 
- **add**: adding new files/directories

**Body**  - Use to explain what and why you have done something. In most cases, you can leave out details about how a change has been made.
- Separate subject from body with a blank line
- Capitalize each paragraph
- A longer commit body MAY be provided after the short descri­ption. The body MUST begin one blank line after the descri­ption.



# How is our team working on adding new feature 

- Asses the current state of the project and prioritize the features to be developed based on importance, complexity and necessary resource allocation
- Split the high priority feature into smaller units and create related issues 
- Keep de-prioritized features in the backlog in case we will have enough time for them in the future
- Assign issues fairly
- Keep track of the progress frequently by checking the related branches and by attending stand up meetings
- Close issues once feature is fully developed (by meeting the Definition of Done)
- Add release tags and add them when closing the merge requests


# How is our team working to fix unexpected behavior on existing features?
- The severity of the bug is assessed when a team member(s) recognizes any unexpected behaviour in the already existing features.
- Along with the severity, we will also discuss the desired or expected behavior, since it may not be clear at first sight
- Once the severity and desired outcome has been discussed, an issue will be opened on GitLab and team members will be immediately allocated to them
- Once the expected behavior is met, a merge request will be opened and the issue will be closed after review


## Authors and acknowledgment
Gabriela Istrate - @istrate 

Ergi Senja - @ergi

Ansis Plepis - @ansis

Jens Pehrsson - @jenspeh

Aditya Nair - @adityana


## License
This project uses license GNU GPLv3.
