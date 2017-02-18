angular.module('comment', [])
.controller('MainCtrl', [
  '$scope', '$http',
  function($scope,$http){
    $scope.test = 'Hello world!';
    $scope.comments = [
    ];
	$scope.predicate = 'title';
	$scope.reverse = true;
	$scope.order = function(predicate) {
		$scope.reverse = ($scope.predicate === predicate) ?
			!$scope.reverse : false;
		$scope.predicate = predicate;
	};
    $scope.addComment = function() {
	if($scope.formContent === '') {return;}
	console.log('In add Comment');
	$scope.create({
		title: $scope.formContent,
		upvote: 0,
	});
	$scope.formContent='';
    };
	$scope.upvote = function(comment) {
		return $http.put('/comments/' + comment._id + '/upvote')
			.success(function(data){
			console.log("upvote worked");
			comment.upvotes += 1;
		});
	}

    $scope.incrementUpvotes = function(comment) {
    	$scope.upvote(comment);
	};

	$scope.decrementUpvotes = function(comment) {
		$scope.downvote(comment)
	};

	$scope.downvote = function(comment){
		return $http.put('/comments/' + comment._id + '/downvote')
			.success(function(data) {
				console.log("donwvote worked");
				comment.upvotes -= 1;
			});
	}


	$scope.getAll = function() {
		return $http.get('/comments').success(function(data){
			angular.copy(data,$scope.comments);
		});
	};
	$scope.getAll();

	$scope.create = function(comment) {
		return $http.post('\comments', comment).success(function(data){
			$scope.comments.push(data);
		});
	}
	

  }
]);

